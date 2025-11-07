#include "ClientHandler.h"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

ClientHandler::ClientHandler(int fd, Event& event, const Router& router)
    : fd_(fd), event_(event), router_(router) {
    written_ = 0;
    len_ = 0;
}

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

void ClientHandler::on_readable() {  // NOLINT
    len_ = ::read(fd_, buf_, sizeof(buf_));
    if (len_ < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;  // no data now
        on_close();
        return;
    }
    if (len_ == 0) {  // EOF
        on_close();
        return;
    }
    written_ = 0;
    event_.mod(fd_, EPOLLOUT);
}

void ClientHandler::on_writable() {
    ssize_t ret;
    ret = ::write(fd_, buf_ + written_, len_ - written_);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;  // try again later
        throw std::runtime_error("write failed: " +
                                 std::string(strerror(errno)));
    }
    written_ += ret;
    if (written_ == len_) {
        len_ = 0;
        written_ = 0;
        event_.mod(fd_, EPOLLIN);
    }
}
