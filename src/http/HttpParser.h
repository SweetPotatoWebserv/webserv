#pragma once

#include <netdb.h>

#include <ctime>
#include <sstream>

#include "../core/Common.h"
#include "../core/String.h"

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
    std::string transfer_encoding_;
    // text/html; charset=UTF-8
    std::string content_type_;
    HttpCommonHeader();
};

struct HttpRequest {
    HttpCommonHeader header_;
    Method method_;
    HostHeader host_;
    UriPath request_target_;
    std::string body_;
    std::string version_;
};

struct HttpResponse {
    HttpCommonHeader header_;
    int status_code_;
    std::string message_;
    HttpDate date_;
    std::string location_;
    std::string body_;
    std::string version_;
    static ssize_t send_response(int client_fd, HttpResponse& response);
    void clear();
};

class HttpParser {
   public:
    static HttpRequest http_request_parse(const std::string& buffer);

   private:
    static const int HOST_ONLY = 2;
    static const int MAX_HOST_FIELD_NUM = 3;
    static const int REQUEST_LINE_NUM = 3;
    static std::vector<std::string> split_path(const std::string& target_path);
    static std::string::size_type request_line_parse(const std::string& buffer,
                                                     HttpRequest& request);
    static std::string::size_type header_section_parse(
        const std::string& buffer, HttpRequest& request,
        std::string::size_type header_section_start);
    static void header_section_host_parse(
        const std::vector<std::string>& header_field, HttpRequest& request);
    static void body_section_parse(const std::string& buffer,
                                   HttpRequest& request,
                                   std::string::size_type body_section_start);

    static std::string parse_chunked(const std::string& data);
};
