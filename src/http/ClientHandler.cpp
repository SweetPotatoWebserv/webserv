#include "ClientHandler.h"

#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <cctype>
#include <cstring>
#include <ctime>

#include "../core/Fd.h"
#include "../core/String.h"
#include "ClientHandlerManager.h"
#include "HttpException.h"
#include "ResponseFactory.h"
#include "Router.h"

const char* const ClientHandler::TRANSFER_ENCODING_CHUNKED_END = "0\r\n\r\n";

ClientHandler::ClientHandler(int fd, Event& event, Router& router,
                             ServerConfig server_config)
    : fd_(fd),
      event_(event),
      router_(router),
      server_config_(server_config), // NOLINT
      should_close_(false) {}

bool ClientHandler::is_request_timeout() const {
    std::time_t now = std::time(NULL);
    return (std::difftime(now, accept_time_.getTime()) >= TIMEOUT_SEC);
}

bool ClientHandler::is_cgi_timeout() const {
    // CGIが実行中でなければ無視
    if (cgi_session_.pid == -1) {
        return false;
    }

    std::time_t now = std::time(NULL);
    return (std::difftime(now, cgi_session_.startTime) >= TIMEOUT_SEC);
}

void ClientHandler::handle_cgi_timeout() {
    kill(cgi_session_.pid, SIGKILL);
    waitpid(cgi_session_.pid, NULL, 0);

    cgi_session_.pid = -1;
    handle_cgi_error(HttpStatus::RequestTimeout);
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
    should_close_ = true;
}

void ClientHandler::cleanup() {
    if (cgi_session_.readFd != -1) {
        event_.del(cgi_session_.readFd);
        fd::Fd::close(cgi_session_.readFd);
    }
    if (cgi_session_.writeFd != -1) {
        event_.del(cgi_session_.writeFd);
        fd::Fd::close(cgi_session_.writeFd);
    }
    if (cgi_session_.pid != -1) {
        kill(cgi_session_.pid, SIGKILL);
        waitpid(cgi_session_.pid, NULL, 0);
        cgi_session_.pid = -1;
    }
    event_.del(fd_);
    fd::Fd::close(fd_);
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
        return;
    }
    // 接続が閉じられた
    if (len == 0) {
        on_close();
        return;
    }
    buffer_.append(buf, len);
    if (ClientHandler::is_request_ready(buffer_)) {
        HttpException exception(HttpStatus::OK);
        RouteInfo info;
        try {
            request_ = HttpParser::http_request_parse(buffer_);
            info = router_.route(server_config_, request_);
        } catch (const HttpException& e) {
            exception = e;
        }

        if (exception.status_code() == HttpStatus::OK &&
            ResponseFactory::is_cgi(request_, info)) {
            try {
                cgi_session_ = cgi_process_.startCgi(request_, info);
                // 読み込み監視
                event_.add(
                    cgi_session_.readFd, EPOLLIN,
                    reinterpret_cast<EventCallback>(ClientHandler::on_event),
                    this);

                // 書き込み監視
                if (!cgi_session_.bodyBuffer.empty()) {
                    event_.add(cgi_session_.writeFd, EPOLLOUT,
                               reinterpret_cast<EventCallback>(
                                   ClientHandler::on_event),
                               this);
                } else {
                    fd::Fd::close(cgi_session_.writeFd);
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
        return;
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

    if (cgi_session_.pid != -1) {
        kill(cgi_session_.pid, SIGKILL);
        waitpid(cgi_session_.pid, NULL, 0);
        cgi_session_.pid = -1;
    }

    // セッションリセット
    cgi_session_ = CgiSession();
}

void ClientHandler::on_cgi_read() {
    int fd = cgi_session_.readFd;
    if (fd == -1) return;
    char buf[BUFFER_SIZE];

    ssize_t ret = read(fd, buf, sizeof(buf));
    if (ret > 0) {
        cgi_session_.responseBuffer.append(buf, ret);
    } else if (ret == 0) {
        finish_cgi_process();
        return;
    } else {
        if (cgi_session_.readFd != -1) {
            event_.del(cgi_session_.readFd);
            fd::Fd::close(cgi_session_.readFd);
            cgi_session_.readFd = -1;
        }
        if (cgi_session_.writeFd != -1) {
            event_.del(cgi_session_.writeFd);
            fd::Fd::close(cgi_session_.writeFd);
            cgi_session_.writeFd = -1;
        }

        handle_cgi_error(HttpStatus::InternalServerError);
        return;
    }
}

void ClientHandler::finish_cgi_process() {
    int fd = cgi_session_.readFd;

    if (fd != -1) {
        event_.del(fd);
        fd::Fd::close(fd);
        cgi_session_.readFd = -1;
    }
    if (cgi_session_.writeFd != -1) {
        event_.del(cgi_session_.writeFd);
        fd::Fd::close(cgi_session_.writeFd);
        cgi_session_.writeFd = -1;
    }

    int status;
    if (cgi_session_.pid != -1) {
        waitpid(cgi_session_.pid, &status, 0);
        cgi_session_.pid = -1;
    } else {
        return;
    }

    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            std::cerr << "CGI Error: Process exited with code " << exit_code
                      << "\n";

            HttpException exception(HttpStatus::InternalServerError);
            response_ = ResponseFactory::make(request_, RouteInfo(), exception);
            event_.mod(fd_, EPOLLOUT);
            cgi_session_ = CgiSession();
            return;
        }
    } else if (WIFSIGNALED(status)) {
        std::cerr << "CGI Error: Process terminated by signal\n";

        HttpException exception(HttpStatus::InternalServerError);
        response_ = ResponseFactory::make(request_, RouteInfo(), exception);
        event_.mod(fd_, EPOLLOUT);
        cgi_session_ = CgiSession();
        return;
    }

    if (cgi_session_.responseBuffer.empty()) {
        std::cerr << "CGI Error: Empty response\n";

        HttpException exception(HttpStatus::InternalServerError);
        response_ = ResponseFactory::make(request_, RouteInfo(), exception);
        event_.mod(fd_, EPOLLOUT);
        cgi_session_ = CgiSession();
        return;
    }

    CgiProcess::parseCgiResponse(response_, cgi_session_.responseBuffer);

    if (request_.method_ == MethodHEAD) {
        response_.body_.clear();
    }

    event_.mod(fd_, EPOLLOUT);

    cgi_session_ = CgiSession();
}
