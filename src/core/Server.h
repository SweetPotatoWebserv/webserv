#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "../event/event.h"
#include "../http/Router.h"
#include "common.h"

class Socket {
   public:
    static Socket listen_tcp(const std::string& host, uint16_t port);

   private:
    Socket();
    ~Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket& rhs);
    int fd_;
};

class Server {
   public:
    Server(std::string host, uint16_t port, EventLoop& loop, Router& router);
    void start();

   private:
    EventLoop& loop_;
    Router& route_;
    int listen_fd_;
    static void on_acceptable(int fd, uint32_t events, void* self);
};
