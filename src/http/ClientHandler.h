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
    static bool is_request_ready(const std::string& buffer);
    static bool is_complete_content_length(
        const std::string& buffer, const std::string& message_head,
        std::string::size_type content_length_pos);
    static bool is_complete_transfer(
        const std::string& message_head,
        std::string::size_type transfer_encoding_pos);

   private:
    int fd_;
    std::string buffer_;
    Event& event_;
    const Router& router_;
    HttpRequest request_;
    HttpResponse response_;
    static const int BUFFER_SIZE = 4096;
    static const int RECV_FLG = 0;
    static const int CONTENT_LENGTH_LEN = 15;
    static const int TRANSFER_ENCODING_LEN = 18;
};
