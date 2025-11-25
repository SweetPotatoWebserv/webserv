#include "Event.h"

#include <sys/epoll.h>

#include <cstring>
#include <stdexcept>

#include "../http/ClientHandler.h"

Event::Event() {
    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1) {
        throw std::runtime_error("epoll_create: " +
                                 std::string(std::strerror(errno)));
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
    ev.data.ptr = data;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        delete data;
        throw std::runtime_error("epoll_ctl ADD failed: " +
                                 std::string(std::strerror(errno)));
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
    ev.data.ptr = data;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl MOD failed: " +
                                 std::string(std::strerror(errno)));
    }
}

void Event::del(int fd) {  // NOLINT
    std::map<int, EventData*>::iterator iter = registry_.find(fd);
    if (iter == registry_.end()) return;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
        throw std::runtime_error("epoll_ctl DEL failed: " +
                                 std::string(std::strerror(errno)));
    }
    delete iter->second;
    registry_.erase(iter);
}

void Event::run() {  // NOLINT
    struct epoll_event events[MAX_EVENTS];

    for (;;) {
        int number_of_fd = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000);
        if (number_of_fd == -1) {
            throw std::runtime_error("epoll_wait failed:" +
                                     std::string(std::strerror(errno)));
        }
        for (int i = 0; i < number_of_fd; ++i) {
            EventData* data = static_cast<EventData*>(events[i].data.ptr);
            data->callback(data->fd, events[i].events, data->user);
        }
        ClientHandler::check_timeout_all();
    }
}

Event::~Event() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
    }
}
