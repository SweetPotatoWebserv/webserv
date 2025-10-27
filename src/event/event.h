#pragma once

#include <sys/epoll.h>

class EventLoop {
   public:
    void add(int fd, uint32_t events, void* user);
    void mod(int fd, uint32_t events, void* user);
    void del(int fd);
    void run();

   private:
    static const uint32_t EVENT_READ = EPOLLIN;
    static const uint32_t EVENT_WRITE = EPOLLOUT;
    static const uint32_t EVENT_ERR = EPOLLERR;
    static const uint32_t EVENT_HUP = EPOLLHUP;
};
