#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

#include "../core/Common.h"

class Event {
   public:
    void add(int fd, uint32_t events, void* user);
    void mod(int fd, uint32_t events, void* user);
    void del(int fd);
    Event init_listen(const Socket& listen, void* user, uint32_t events);
    void run();
    Event();
    ~Event();

   private:
    Event(const Event& other);
    Event& operator=(const Event& rhs);
    int epoll_fd_;
    static const uint32_t EVENT_READ = EPOLLIN;
    static const uint32_t EVENT_WRITE = EPOLLOUT;
    static const uint32_t EVENT_ERR = EPOLLERR;
    static const uint32_t EVENT_HUP = EPOLLHUP;
    static const int MAX_EVENTS = 10000;
};
