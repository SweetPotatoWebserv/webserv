#pragma once
#include "HttpParser.h"

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
    static HttpResponse render_error(
        int status_code, const std::map<int, ErrorPageDirective>& error_pages,
        const ServerConfig& servers);
};
