#pragma once

#include <netdb.h>

#include <ctime>
#include <sstream>

#include "../core/Common.h"

class UriPath {
   private:
    std::string path_;
    std::string query_string_;
};

class HttpDate {
   public:
    static HttpDate from_string(const std::string& s);
    std::string to_string() const;

   private:
    std::time_t timestamp_;
};

class HttpCommonHeader {
    // Content-Length: 512
    std::size_t content_length_;
    // Transfer-Encoding: chunked
    std::vector<std::string> transfer_encoding_;
    // text/html; charset=UTF-8
    std::string content_type_;
};

class HttpRequest : public HttpCommonHeader {
    Method method_;
    HostHeader host_;
    UriPath request_target_;
};

class HttpResponse : public HttpCommonHeader {
    int status_code_;
    std::string message_;
    HttpDate date_;
    std::string location_;
};

class HttpParser {
   public:
    void http_request_parse();

   private:
    HttpRequest request_;
};
