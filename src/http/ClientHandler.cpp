#include "ClientHandler.h"

#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "../core/String.h"
#include "HttpException.h"

const char* const ClientHandler::TRANSFER_ENCODING_CHUNKED_END = "0\r\n\r\n";

ClientHandler::ClientHandler(int fd, Event& event, Router& router)
    : fd_(fd), event_(event), router_(router) {}

void ClientHandler::on_event(int fd, uint32_t event, void* self) {  // NOLINT
    static_cast<void>(fd);
    ClientHandler* handler = static_cast<ClientHandler*>(self);
    if (event & (EPOLLHUP | EPOLLERR)) {
        handler->on_close();
    } else if (event & EPOLLIN) {
        handler->on_readable();
    } else if (event & EPOLLOUT) {
        handler->on_writable();
    }
}

void ClientHandler::on_close() {
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

void ClientHandler::on_readable() {  // NOLINT
    char buf[BUFFER_SIZE];
    ssize_t len = ::recv(fd_, buf, sizeof(buf), RECV_FLG);
    // recv の失敗
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
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
        try {
            request_ = HttpParser::http_request_parse(buffer_);
        } catch (const HttpException& e) {
            exception = e;
        }
        response_ = router_.create_response(request_, exception);
        event_.mod(fd_, EPOLLOUT);
    }
}

void ClientHandler::on_writable() {  // NOLINT
    ssize_t ret = HttpResponse::send_response(fd_, response_);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
    }
    buffer_.clear();
    response_.clear();
    event_.mod(fd_, EPOLLIN);
}
