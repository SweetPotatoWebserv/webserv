#pragma once
#include "../event/Event.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "Router.h"

class ClientHandler {
   public:
    ClientHandler(int fd, Event& event, const Router& router);
    static void on_event(int fd, uint32_t event, void* self);
    void on_close();
    void on_readable();
    void on_writable();
    static bool is_request_ready(const std::string& buffer);
    static bool is_complete_content_length(
        const std::string& buffer, const std::string& message_head,
        const std::vector<std::string>& content_length);
    static bool is_complete_transfer(
        const std::string& buffer, const std::string& message_head,
        const std::vector<std::string>& transfer_encoding);
    static const char* const TRANSFER_ENCODING_CHUNKED_END;

   private:
    int fd_;
    std::string buffer_;
    Event& event_;
    const Router& router_;
    HttpRequest request_;
    HttpResponse response_;
    static const int BUFFER_SIZE = 4096;
    static const int RECV_FLG = 0;
};
