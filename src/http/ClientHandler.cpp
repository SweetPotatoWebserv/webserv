#include "ClientHandler.h"

#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "HttpException.h"

const char* const ClientHandler::CONTENT_LENGTH_WITH_COLON = "content-length:";
const char* const ClientHandler::TRANSFER_ENCODING_WITH_COLON =
    "transfer-encoding:";
const char* const ClientHandler::TRANSFER_ENCODING_CHUNKED_END = "0\r\n\r\n";

ClientHandler::ClientHandler(int fd, Event& event, const Router& router)
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

bool ClientHandler::is_request_ready(const std::string& buffer) {
    std::string::size_type message_head_end = buffer.find(HTTP_HEADER_END);
    if (message_head_end == std::string::npos) return false;
    std::string message_head = buffer.substr(0, message_head_end);
    std::transform(message_head.begin(), message_head.end(),
                   message_head.begin(), ::tolower);

    std::string::size_type transfer_encoding_pos =
        message_head.find(TRANSFER_ENCODING_WITH_COLON);
    if (transfer_encoding_pos != std::string::npos)
        return is_complete_transfer(buffer, message_head,
                                    transfer_encoding_pos);
    std::string::size_type content_length_pos =
        message_head.find(CONTENT_LENGTH_WITH_COLON);
    if (content_length_pos != std::string::npos)
        return is_complete_content_length(buffer, message_head,
                                          content_length_pos);
    return true;
}

bool ClientHandler::is_complete_transfer(
    const std::string& buffer, const std::string& message_head,  // NOLINT
    std::string::size_type transfer_encoding_pos) {
    transfer_encoding_pos += TRANSFER_ENCODING_WITH_COLON_LEN;
    while (transfer_encoding_pos < message_head.size() &&
           std::isspace(message_head[transfer_encoding_pos]))
        transfer_encoding_pos++;
    std::string::size_type transfer_encoding_end =
        message_head.find(HTTP_LINE_END, transfer_encoding_pos);
    std::string transfer_encoding_value = message_head.substr(
        transfer_encoding_pos, transfer_encoding_end - transfer_encoding_pos);
    if (transfer_encoding_value.find(CHUNKED) == std::string::npos) {
        return true;
    }
    std::string::size_type header_end_in_buffer =
        message_head.size() + HTTP_HEADER_END_LEN;
    return buffer.find(TRANSFER_ENCODING_CHUNKED_END, header_end_in_buffer) !=
           std::string::npos;
}

bool ClientHandler::is_complete_content_length(
    const std::string& buffer, const std::string& message_head,
    std::string::size_type content_length_pos) {
    size_t content_length_value = 0;
    content_length_pos += CONTENT_LENGTH_WITH_COLON_LEN;
    std::string::size_type message_head_size = message_head.size();
    while (content_length_pos < message_head_size &&
           std::isspace(message_head[content_length_pos]))
        content_length_pos++;
    std::string::size_type content_length_header_end = content_length_pos;
    while (content_length_header_end < message_head_size &&
           std::isdigit(message_head[content_length_header_end]))
        content_length_header_end++;
    content_length_value =
        std::strtoul(message_head
                         .substr(content_length_pos,
                                 content_length_header_end - content_length_pos)
                         .c_str(),
                     NULL, DECIMAL);

    // リクエストが完全に届いたのか判定する
    size_t total_len =
        message_head_size + HTTP_HEADER_END_LEN + content_length_value;
    return buffer.size() >= total_len;
}

void ClientHandler::on_readable() {  // NOLINT
    char buf[BUFFER_SIZE];
    ssize_t len = 0;
    len = ::recv(fd_, buf, sizeof(buf), RECV_FLG);
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
        // request_ = HttpParser::http_request_parse(buffer_);
        // try {
        // } catch (const HttpException& e) {
        //     sendErrorResponse(e.status_code());
        // }
        event_.mod(fd_, EPOLLOUT);
    }
}

void ClientHandler::on_writable() {  // NOLINT
    std::cout << "method: " << request_.method_ << '\n';
    std::cout << "path: " << request_.request_target_.path_ << '\n';
    // ssize_t ret;
    // ret = ::write(fd_, buf_ + written_, len_ - written_);
    // if (ret == -1) {
    //     if (errno == EAGAIN || errno == EWOULDBLOCK) return;  // try again
    //     later throw std::runtime_error("write failed: " +
    //                              std::string(strerror(errno)));
    // }
    // written_ += ret;
    // if (written_ == len_) {
    //     len_ = 0;
    //     written_ = 0;
    //     std::memset(buf_, 0, sizeof(buf_));
    //     event_.mod(fd_, EPOLLIN);
    // }
}
