#pragma once
#include "../cgi/handler_cgi.h"
#include "../event/Event.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "ResponseFactory.h"
#include "Router.h"

class ClientHandler {
   public:
    ClientHandler(int fd, Event& event, Router& router,
                  ServerConfig server_config);
    static void on_event(int fd, uint32_t event, void* self);
    void on_close();
    void on_readable();
    void on_writable();
    void on_cgi_read();
    void on_cgi_write();
    static void check_timeout_all();
    static bool is_request_ready(const std::string& buffer);
    static bool is_complete_content_length(
        const std::string& buffer, const std::string& message_head,
        const std::vector<std::string>& content_length);
    static bool is_complete_transfer(
        const std::string& buffer, const std::string& message_head,
        const std::vector<std::string>& transfer_encoding);
    static const char* const TRANSFER_ENCODING_CHUNKED_END;

    void handle_cgi_error(int status_code);

   private:
    int fd_;
    Event& event_;
    Router& router_;
    ServerConfig server_config_;
    std::string buffer_;
    HttpRequest request_;
    HttpResponse response_;
    CgiProcess cgi_process_;
    CgiSession cgi_session_;
    void check_cgi_timeout();
    static std::vector<ClientHandler*> all_handlers_;
    static const int BUFFER_SIZE = 4096;
    static const int RECV_FLG = 0;
    static const int CGI_TIMEOUT_SEC = 5;
};
