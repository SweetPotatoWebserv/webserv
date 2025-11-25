#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

#include "../core/Common.h"

typedef void (*EventCallback)(int, uint32_t, void*);
class Event {
   public:
    void add(int fd, uint32_t events, EventCallback callback, void* user);
    void mod(int fd, uint32_t events);
    void del(int fd);
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
    static const int MONITOR_MSEC = 1000;
    struct EventData {
        int fd;
        uint32_t events;
        EventCallback callback;
        void* user;
    };
    std::map<int, EventData*> registry_;
};
