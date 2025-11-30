#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <stdexcept>

#include "../event/Event.h"
#include "../http/ClientHandlerManager.h"
#include "../http/Router.h"
#include "Common.h"
#include "Socket.h"

class Server {
   public:
    Server(Event& event, Router& router, const ServerConfig& config, ClientHandlerManager& manager);
    void start();
    static void on_timeout(void* self);

   private:
    const fd::Socket listen_;
    Event& event_;
    Router& router_;
    const ServerConfig server_config_;
    ClientHandlerManager& manager_;
    static void on_acceptable(int fd, uint32_t event, void* self);
};
