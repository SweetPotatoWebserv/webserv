#pragma once
#include "../event/Event.h"
#include "HttpParser.h"
#include "Router.h"

class ClientHandler {
   public:
    ClientHandler(int fd, Event& event, Router& router);
    static void on_event(int fd, uint32_t event, void* self);
    void on_close();
    void on_readable();
    void on_writable();

   private:
    int fd_;
    std::string buffer_;
    Event& event_;
    Router& router_;
    HttpRequest request_;
    HttpResponse response_;
};
