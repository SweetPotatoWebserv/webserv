#include "Router.h"

#include <vector>

Router::Router(const HttpConfig& config) : config_(config) {}

HttpResponse Router::create_response(  // NOLINT
    const HttpRequest& request) const {
    HttpResponse response;
    response.header_.content_length_ = request.header_.content_length_;
    response.header_.content_type_ = request.header_.content_type_;
    return response;
}

// struct HttpCommonHeader {
//     // Content-Length: 512
//     std::size_t content_length_;
//     // Transfer-Encoding: chunked
//     std::vector<std::string> transfer_encoding_;
//     // text/html; charset=UTF-8
//     std::string content_type_;
// };
//  struct HttpResponse {
//      HttpCommonHeader header_;
//      int status_code_;
//      std::string message_;
//      HttpDate date_;
//      std::string location_;
//  };
