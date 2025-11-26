#include "ClientHandler.h"

#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <stdexcept>

#include "../core/String.h"
#include "HttpException.h"
#include "ResponseFactory.h"
#include "Router.h"

std::vector<ClientHandler*>& ClientHandler::getAllHandlers() {
    static std::vector<ClientHandler*> handlers;
    return handlers;
}

const char* const ClientHandler::TRANSFER_ENCODING_CHUNKED_END = "0\r\n\r\n";

ClientHandler::ClientHandler(int fd, Event& event, Router& router,
                             ServerConfig server_config)
    : fd_(fd),
      event_(event),
      router_(router),
      server_config_(server_config) {  // NOLINT
    getAllHandlers().push_back(this);
}

void ClientHandler::check_timeout_all() {
    std::vector<ClientHandler*>& handlers = getAllHandlers();
    for (std::vector<ClientHandler*>::iterator it = handlers.begin();
         it != handlers.end(); ++it) {
        (*it)->check_cgi_timeout();
    }
}

void ClientHandler::check_cgi_timeout() {
    // CGIが実行中でなければ無視
    if (cgi_session_.pid == -1) {
        return;
    }

    std::time_t now = std::time(NULL);
    if (std::difftime(now, cgi_session_.startTime) >= CGI_TIMEOUT_SEC) {
        kill(cgi_session_.pid, SIGKILL);
        waitpid(cgi_session_.pid, NULL, 0);

        // プロセス回収済みなのでPIDをリセット
        // これで on_cgi_read 側での二重 waitpid を防ぐ
        cgi_session_.pid = -1;

        // 2. パイプのクローズと監視削除
        if (cgi_session_.readFd != -1) {
            event_.del(cgi_session_.readFd);
            close(cgi_session_.readFd);
            cgi_session_.readFd = -1;
        }
        if (cgi_session_.writeFd != -1) {
            event_.del(cgi_session_.writeFd);
            close(cgi_session_.writeFd);
            cgi_session_.writeFd = -1;
        }

        handle_cgi_error(HttpStatus::RequestTimeout);
    }
}

void ClientHandler::on_event(int fd, uint32_t event, void* self) {  // NOLINT
    ClientHandler* handler = static_cast<ClientHandler*>(self);

    // CGI 読み込み用FDからのイベント
    if (handler->cgi_session_.readFd != -1 &&
        fd == handler->cgi_session_.readFd) {
        handler->on_cgi_read();
        return;
    }

    // CGI 書き込み用FDからのイベント
    if (handler->cgi_session_.writeFd != -1 &&
        fd == handler->cgi_session_.writeFd) {
        handler->on_cgi_write();
        return;
    }

    if (event & (EPOLLHUP | EPOLLERR)) {
        handler->on_close();
    } else if (event & EPOLLIN) {
        handler->on_readable();
    } else if (event & EPOLLOUT) {
        handler->on_writable();
    }
}

void ClientHandler::on_close() {
    std::vector<ClientHandler*>& handlers = getAllHandlers();
    for (std::vector<ClientHandler*>::iterator it = handlers.begin();
         it != handlers.end(); ++it) {
        if (*it == this) {
            handlers.erase(it);
            break;
        }
    }
    if (cgi_session_.readFd != -1) {
        event_.del(cgi_session_.readFd);
        close(cgi_session_.readFd);
    }
    if (cgi_session_.writeFd != -1) {
        event_.del(cgi_session_.writeFd);
        close(cgi_session_.writeFd);
    }
    event_.del(fd_);
    ::close(fd_);
    delete this;
}

bool ClientHandler::is_request_ready(const std::string& buffer) {
    std::string::size_type message_head_end = buffer.find(HTTP_HEADER_END);
    if (message_head_end == std::string::npos) return false;
    std::string message_head =
        buffer.substr(0, message_head_end + HTTP_HEADER_END_LEN);
    std::transform(message_head.begin(), message_head.end(),
                   message_head.begin(), ::tolower);

    std::vector<std::string> found_field;
    if (search_header_field(message_head, TRANSFER_ENCODING, found_field))
        return is_complete_transfer(buffer, message_head, found_field);
    if (search_header_field(message_head, CONTENT_LENGTH, found_field)) {
        return is_complete_content_length(buffer, message_head, found_field);
    }
    return true;
}

bool ClientHandler::is_complete_transfer(
    const std::string& buffer, const std::string& message_head,
    const std::vector<std::string>& transfer_encoding) {
    if (transfer_encoding[1].find(CHUNKED) == std::string::npos) {
        return true;
    }
    return buffer.find(TRANSFER_ENCODING_CHUNKED_END, message_head.size()) !=
           std::string::npos;
}

bool ClientHandler::is_complete_content_length(
    const std::string& buffer, const std::string& message_head,
    const std::vector<std::string>& content_length) {
    // リクエストが完全に届いたのか判定する
    size_t total_len =
        message_head.size() + strtoul(content_length[1].c_str(), NULL, DECIMAL);
    return buffer.size() >= total_len;
}

void ClientHandler::on_readable() {
    char buf[BUFFER_SIZE];
    ssize_t len = ::recv(fd_, buf, sizeof(buf), RECV_FLG);
    if (len == -1) {
        on_close();
        throw std::runtime_error("recv() failed: " +
                                 std::string(strerror(errno)));
    }
    // 接続が閉じられた
    if (len == 0) {
        on_close();
        return;
    }
    buffer_.append(buf, len);
    if (ClientHandler::is_request_ready(buffer_)) {
        HttpException exception(HttpStatus::OK);
        // TODO RouterInfo が適切に初期化されるか確認する
        RouteInfo info;
        try {
            request_ = HttpParser::http_request_parse(buffer_);
            info = router_.route(server_config_, request_);
        } catch (const HttpException& e) {
            exception = e;
        }

        bool is_cgi = false;
        if (exception.status_code() == HttpStatus::OK) {
            is_cgi = ResponseFactory::is_cgi(request_, info);
        }

        if (is_cgi) {
            try {
                std::cout << "this is cgi request\n";
                cgi_session_ = cgi_process_.startCgi(request_, info);

                // 読み込み監視
                event_.add(
                    cgi_session_.readFd, EPOLLIN,
                    reinterpret_cast<EventCallback>(ClientHandler::on_event),
                    this);

                // 書き込み監視（ボディがある場合）
                if (!cgi_session_.bodyBuffer.empty()) {
                    event_.add(cgi_session_.writeFd, EPOLLOUT,
                               reinterpret_cast<EventCallback>(
                                   ClientHandler::on_event),
                               this);
                } else {
                    close(cgi_session_.writeFd);
                    cgi_session_.writeFd = -1;
                }

                // CGIが終わるまでクライアントへの書き込みは待機
                return;
            } catch (const HttpException& e) {
                exception = e;
            } catch (const std::exception& e) {
                exception = HttpException(HttpStatus::InternalServerError);
            }
        }
        response_ = ResponseFactory::make(request_, info, exception);
        event_.mod(fd_, EPOLLOUT);
    }
}

void ClientHandler::on_writable() {
    ssize_t ret = HttpResponse::send_response(fd_, response_);
    if (ret == -1) {
        on_close();
        throw std::runtime_error("write() failed: " +
                                 std::string(strerror(errno)));
    }
    buffer_.clear();
    response_.clear();
    cgi_session_ = CgiSession();
    event_.mod(fd_, EPOLLIN);
}

void ClientHandler::on_cgi_write() {
    int fd = cgi_session_.writeFd;
    const std::string& body = cgi_session_.bodyBuffer;
    size_t& sent = cgi_session_.sentBytes;

    ssize_t ret = write(fd, body.c_str() + sent, body.size() - sent);

    if (ret > 0) {
        sent += ret;
        if (sent >= body.size()) {
            event_.del(fd);
            close(fd);
            cgi_session_.writeFd = -1;
        }
    } else {
        // エラー時
        event_.del(fd);
        close(fd);
        cgi_session_.writeFd = -1;
    }
}

void ClientHandler::handle_cgi_error(int status_code) {
    // エラーページを生成
    HttpException exception(status_code);
    response_ = ResponseFactory::make(request_, RouteInfo(), exception);

    // クライアントへ送信フェーズへ移行
    event_.mod(fd_, EPOLLOUT);

    // セッションリセット
    cgi_session_ = CgiSession();
}

void ClientHandler::on_cgi_read() {
    int fd = cgi_session_.readFd;
    char buf[BUFFER_SIZE];
    ssize_t ret = read(fd, buf, sizeof(buf));

    if (ret > 0) {
        cgi_session_.responseBuffer.append(buf, ret);
    } else if (ret == 0) {
        // --- EOF: CGIプロセス終了 ---

        event_.del(fd);
        close(fd);
        cgi_session_.readFd = -1;
        if (cgi_session_.writeFd != -1) {
            event_.del(cgi_session_.writeFd);
            close(cgi_session_.writeFd);
            cgi_session_.writeFd = -1;
        }

        int status;

        if (cgi_session_.pid != -1) {
            waitpid(cgi_session_.pid, &status, 0);
            cgi_session_.pid = -1;
        } else {
            // すでに kill 済み（タイムアウトなど）の場合の処理
            // ここに来る＝タイムアウト後にEOFイベントが遅れて来たということなので
            // return する
            return;
        }
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                std::cerr << "CGI Error: Process exited with code " << exit_code
                          << "\n";
                handle_cgi_error(HttpStatus::InternalServerError);
                return;
            }
        } else if (WIFSIGNALED(status)) {
            std::cerr << "CGI Error: Process terminated by signal\n";
            handle_cgi_error(HttpStatus::InternalServerError);
            return;
        }

        if (cgi_session_.responseBuffer.empty()) {
            std::cerr << "CGI Error: Empty response\n";
            handle_cgi_error(HttpStatus::InternalServerError);
            return;
        }

        CgiProcess::parseCgiResponse(response_, cgi_session_.responseBuffer);

        response_.header_.content_length_ = response_.body_.size();

        if (request_.method_ == MethodHEAD) {
            response_.body_.clear();
        }

        // クライアントへの送信準備完了
        event_.mod(fd_, EPOLLOUT);

    } else {
        // 異常系 readエラー
        std::cerr << "CGI read failed\n";
        event_.del(fd);
        close(fd);
        cgi_session_.readFd = -1;

        if (cgi_session_.pid != -1) {
            kill(cgi_session_.pid, SIGKILL);
            waitpid(cgi_session_.pid, NULL, 0);
            cgi_session_.pid = -1;
        }
        handle_cgi_error(HttpStatus::InternalServerError);
    }
}
