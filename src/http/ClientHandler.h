#pragma once
#include "../event/Event.h"
#include "HttpParser.h"
#include "Router.h"

class ClientHandler {
   public:
    ClientHandler(int fd, Event& event, const Router& router)
        : fd_(fd), event_(event), router_(router) {}
    static void on_event(uint32_t event, void* self) {
        ClientHandler* handler = static_cast<ClientHandler*>(self);
        if (event & EPOLLIN) handler->on_readable();
        if (event & EPOLLOUT) handler->on_writable();
        if (event & EPOLLHUP | EPOLLERR) handler->on_close();
    }

    void on_close() {
        event_.del(fd_);
        ::close(fd_);
        delete this;
    }
    void on_readable();
    void on_writable();

   private:
    int fd_;
    Event& event_;
    const Router& router_;
    HttpParser parser_;
    HttpRequest request_;
    HttpResponse response_;
};
