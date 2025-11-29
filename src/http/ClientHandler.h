#pragma once
#include "../cgi/handler_cgi.h"
#include "../event/Event.h"
#include "ClientHandlerManager.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "ResponseFactory.h"
#include "Router.h"

class ClientHandler {
   public:
    ClientHandler(int fd, Event& event, Router& router,
                  ServerConfig server_config, ClientHandlerManager manager);
    static const char* const TRANSFER_ENCODING_CHUNKED_END;
    void handle_cgi_error(int status_code);
    const CgiSession& getCgiSession() const { return cgi_session_; }
    void cleanup();
    static void on_event(int fd, uint32_t event, void* self);
    void check_cgi_timeout();
    void check_request_timeout();

   private:
    int fd_;
    Event& event_;
    Router& router_;
    HttpDate accept_time_;
    ServerConfig server_config_;
    std::string buffer_;
    HttpRequest request_;
    HttpResponse response_;
    CgiProcess cgi_process_;
    CgiSession cgi_session_;
    ClientHandlerManager manager_;
    void finish_cgi_process();
    void on_close();
    void on_readable();
    void on_writable();
    void on_cgi_read();
    void on_cgi_write();
    static bool is_request_ready(const std::string& buffer);
    static bool is_complete_content_length(
        const std::string& buffer, const std::string& message_head,
        const std::vector<std::string>& content_length);
    static bool is_complete_transfer(
        const std::string& buffer, const std::string& message_head,
        const std::vector<std::string>& transfer_encoding);
    static const int BUFFER_SIZE = 4096;
    static const int RECV_FLG = 0;
    static const int TIMEOUT_SEC = 5;
};
