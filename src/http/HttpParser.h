#pragma once

#include <netdb.h>

#include <ctime>
#include <sstream>

#include "../core/Common.h"

struct UriPath {
    std::string path_;
    std::string query_string_;
};

class HttpDate {
   public:
    HttpDate from_string(const std::string& s) const;
    std::string to_string() const;

   private:
    std::time_t timestamp_;
};

struct HttpCommonHeader {
    // Content-Length: 512
    std::size_t content_length_;
    // Transfer-Encoding: chunked
    std::vector<std::string> transfer_encoding_;
    // text/html; charset=UTF-8
    std::string content_type_;
};

struct HttpRequest {
    HttpCommonHeader header_;
    Method method_;
    HostHeader host_;
    UriPath request_target_;
    std::string body_;
};

struct HttpResponse {
    HttpCommonHeader header_;
    int status_code_;
    std::string message_;
    HttpDate date_;
    std::string location_;
    std::string body_;
    static ssize_t send_response(int client_fd, HttpResponse& response);
};

class HttpParser {
   public:
    static HttpRequest http_request_parse(const std::string& buffer);
};
