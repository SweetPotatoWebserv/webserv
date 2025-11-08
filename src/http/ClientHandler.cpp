#include "ClientHandler.h"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

ClientHandler::ClientHandler(int fd, Event& event, Router& router)
    : fd_(fd), event_(event), router_(router) {}

void ClientHandler::on_event(int fd, uint32_t event, void* self) {  // NOLINT
    static_cast<void>(fd);
    ClientHandler* handler = static_cast<ClientHandler*>(self);
    if (event & EPOLLIN) handler->on_readable();
    if (event & EPOLLOUT) handler->on_writable();
    if (event & (EPOLLHUP | EPOLLERR)) handler->on_close();
}

void ClientHandler::on_close() {
    event_.del(fd_);
    ::close(fd_);
    delete this;
}

// TODO transfer-encoding にも対応
bool is_request_complete(std::string& buffer) {
    size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;
    }

    std::string header_part = buffer.substr(0, header_end + 4);

    size_t content_length = 0;
    std::string::size_type pos = header_part.find("Content-Length:");
    if (pos != std::string::npos) {
        pos += strlen("Content-Length:");
        while (pos < header_part.size() && std::isspace(header_part[pos]))
            pos++;
        size_t end = pos;
        while (end < header_part.size() && isdigit(header_part[end])) end++;
        content_length = std::strtoul(
            header_part.substr(pos, end - pos).c_str(), NULL, 10);  // NOLINT
    }
    // ヘッダ境界は CRLFCRLF (=4 bytes)
    size_t total_len = header_end + 4 + content_length;
    return buffer.size() >= total_len;
}

void ClientHandler::on_readable() {  // NOLINT
    char buf[4096];                  // NOLINT
    ssize_t len = 0;
    len = ::recv(fd_, buf, sizeof(buf), 0);
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;  // no data now
        on_close();
        return;
    }
    if (len == 0) {  // EOF
        on_close();
        return;
    }
    buffer_.append(buf, len);
    if (is_request_complete(buffer_)) {
        request_ = HttpParser::http_request_parse(buffer_);
        event_.mod(fd_, EPOLLOUT);
    }
}

void ClientHandler::on_writable() {  // NOLINT
    response_ = router_.create_response(request_);
    ssize_t ret = ::write(fd_, response_.body_.c_str(), response_.body_.size());
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
    }
    event_.mod(fd_, EPOLLIN);
}
