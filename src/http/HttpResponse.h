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
};
