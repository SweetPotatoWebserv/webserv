#pragma once
#include "HttpDate.h"
#include "HttpParser.h"
#include "Router.h"

struct HttpResponse {
    HttpCommonHeader header_;
    int status_code_;
    std::string message_;
    HttpDate date_;
    std::string location_;
    std::string body_;
    std::string version_;
    void clear();
    static ssize_t send_response(int client_fd, HttpResponse& response);
    static HttpResponse render_default_error_page(int status_code);
    static HttpResponse render_error(int status_code, const RouteInfo& route);

   private:
    static const int DEFAULT_BUFFER_SIZE = 1024;
};
