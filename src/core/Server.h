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
    Server(Event& event, Router& router,
           const std::string& host = DEFAULT_ADDRESS,
           uint16_t port = DEFAULT_PORT);
    void start();

   private:
    Socket listen_;
    Event& event_;
    Router& router_;
    static void on_acceptable(int fd, uint32_t event, void* self);
};
