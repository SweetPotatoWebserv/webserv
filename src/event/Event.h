#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

#include "../core/Common.h"

typedef void (*EventCallback)(int, uint32_t, void*);
typedef void (*TimeoutCallback)(void* user);

struct EventData {
    int fd;
    uint32_t events;
    EventCallback callback;
    void* user;
};

class Event {
    private:
    static const int MAX_EVENTS = 10000;
    static const int TIMEOUT_MS = 100;

    int epoll_fd_;
    std::map<int, EventData*> registry_;
    TimeoutCallback timeout_callback_;
    void* timeout_user_;

   public:
    void add(int fd, uint32_t events, EventCallback callback, void* user);
    void mod(int fd, uint32_t events);
    void del(int fd);

    void run();

    void set_timeout_callback(TimeoutCallback callback, void* user);
    Event();
    ~Event();

   private:
    Event(const Event& other);
    Event& operator=(const Event& rhs);
};
