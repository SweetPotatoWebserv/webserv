#pragma once
#include "../event/Event.h"
#include "HttpParser.h"
#include "Router.h"

class ClientHandler {
   public:
    ClientHandler(int fd, Event& event, const Router& router);
    static void on_event(int fd, uint32_t event, void* self);
    void on_close();
    void on_readable();
    void on_writable();

   private:
    int fd_;
    Event& event_;
    const Router& router_;
    HttpParser parser_;
    HttpRequest request_;
    HttpResponse response_;

    // TODO echo サーバー用の変数を削除する
    char buf_[1024];  // NOLINT
    ssize_t written_;
    ssize_t len_;
};
