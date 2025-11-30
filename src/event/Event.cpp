#include "Event.h"

#include <sys/epoll.h>

#include <cstring>

#include "../core/Fd.h"

Event::Event() {
    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1) {
        throw std::runtime_error(std::string("epoll_create: ").append(std::strerror(errno)));
    }
}

void Event::add(int fd, uint32_t events, EventCallback callback,  // NOLINT
                void* user) {
    EventData* data = new EventData;
    data->fd = fd;
    data->events = events;
    data->callback = callback;
    data->user = user;

    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        delete data;
        throw std::runtime_error(std::string("epoll_ctl ADD failed: ").append(std::strerror(errno)));
    }
    registry_[fd] = data;
}

void Event::mod(int fd, uint32_t events) {  // NOLINT
    std::map<int, EventData*>::iterator iter = registry_.find(fd);
    if (iter == registry_.end()) return;

    EventData* data = iter->second;
    data->events = events;

    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error(std::string("epoll_ctl MOD failed: ").append(std::strerror(errno)));
    }
}

void Event::del(int fd) {  // NOLINT
    std::map<int, EventData*>::iterator iter = registry_.find(fd);
    if (iter == registry_.end()) return;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
        throw std::runtime_error(std::string("epoll_ctl DEL failed: ").append(std::strerror(errno)));
    }
    delete iter->second;
    registry_.erase(iter);
}

void Event::run() {
    struct epoll_event events[MAX_EVENTS];

    for (;;) {
        int number_of_fd = epoll_wait(epoll_fd_, events, MAX_EVENTS, TIMEOUT_MS);
        if (number_of_fd == -1) {
            throw std::runtime_error(std::string("epoll_wait failed:").append(std::strerror(errno)));
        }
        for (int i = 0; i < number_of_fd; ++i) {
            int fd = events[i].data.fd;
            std::map<int, EventData*>::iterator it = registry_.find(fd);
            if (it == registry_.end()) {
                continue;
            }
            EventData* data = it->second;
            data->callback(data->fd, events[i].events, data->user);
        }

        if (timeout_callback_) {
            timeout_callback_(timeout_user_);
        }
    }
}

Event::~Event() {
    if (epoll_fd_ >= 0) {
        fd::Fd::close(epoll_fd_);
    }
    for (std::map<int, EventData*>::iterator it = registry_.begin();
         it != registry_.end(); ++it) {
        delete it->second;
    }
    registry_.clear();
}

void Event::set_timeout_callback(TimeoutCallback callback, void* user) {
    timeout_callback_ = callback;
    timeout_user_ = user;
}
