#include "Event.h"

#include <sys/epoll.h>

#include <cstring>
#include <stdexcept>

Event::Event() {
    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1) {
        throw std::runtime_error("epoll_create: " +
                                 std::string(std::strerror(errno)));
    }
}

void Event::add(int fd, uint32_t events, void* user) {  // NOLINT
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = user;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl ADD failed: " +
                                 std::string(std::strerror(errno)));
    }
}

void Event::mod(int fd, uint32_t events, void* user) {  // NOLINT
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = user;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl MOD failed: " +
                                 std::string(std::strerror(errno)));
    }
}

void Event::del(int fd) {  // NOLINT
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
        throw std::runtime_error("epoll_ctl DEL failed: " +
                                 std::string(std::strerror(errno)));
    }
}

void Event::run() {  // NOLINT
    struct epoll_event events[MAX_EVENTS];
    for (;;) {
        int number_of_df = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (number_of_df == -1) {
            throw std::runtime_error("epoll_wait failed:" +
                                     std::string(std::strerror(errno)));
        }
        for (int i = 0; i < number_of_df; ++i) {
            // event の処理
        }
    }
}

Event Event::init_listen(const Socket& listen, void* user = 0,
                         uint32_t events = EPOLLIN) {
    Event epoll;
    add(listen.getFd(), events, user);
    return epoll;
}

Event::~Event() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
    }
}
