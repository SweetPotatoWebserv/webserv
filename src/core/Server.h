#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <stdexcept>

#include "../event/Event.h"
#include "../http/Router.h"
#include "Common.h"

class Server {
   public:
    Server(std::string host, uint16_t port, Event& event, Router& router);
    void start();

   private:
    Socket listen_;
    Event& event_;
    Router& route_;
    static void on_acceptable(int fd, uint32_t events, void* self);
};
