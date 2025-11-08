#include "Router.h"

#include <vector>

#include "../core/Common.h"

Router::Router(const HttpConfig& config) : config_(config) {}

ServerConfig* Router::find_server(const HttpRequest& request) {
    // パスを探索
    ServerConfig* matched = NULL;
    for (std::vector<ServerConfig>::iterator servers = config_.servers_.begin();
         servers != config_.servers_.end(); ++servers) {
        if (servers->listens_.port != request.host_.getPort()) continue;
        if (servers->listens_.address != "0.0.0.0" &&
            servers->listens_.address != request.host_.getAddress())
            continue;
        for (std::vector<std::string>::iterator server_names =
                 servers->server_names_.begin();
             server_names != servers->server_names_.end(); ++server_names) {
            if (*server_names == request.host_.getAddress()) {
                matched = &(*servers);
                break;
            }
        }
        if (matched) break;
    }
    if (!matched && !config_.servers_.empty())
        matched = &config_.servers_.front();
    return matched;
}

HttpResponse Router::create_response(const HttpRequest& request) {
    HttpResponse response;
    response.header_.content_length_ = request.header_.content_length_;
    response.header_.content_type_ = request.header_.content_type_;
    // 対象の server を見つける
    ServerConfig* server = find_server(request);
    // 対象のパスを見つける
    LocationConfig* match = NULL;
    for (std::vector<LocationConfig>::iterator locations =
             server->locations_.begin();
         locations != server->locations_.end(); ++locations) {
        if (request.request_target_.path_ == locations->path_) {
            match = &(*locations);
            break;
        }
    }
#include <fcntl.h>

    match->common_config_.root_ = "docs/html/";
    char buf[1024];  // NOLINT
    for (std::vector<std::string>::const_iterator it =
             match->common_config_.index_files_.begin();
         it != match->common_config_.index_files_.end(); ++it) {
        char* pwd = getcwd(NULL, 0);
        std::string path_name =
            std::string(pwd) + match->path_ + match->common_config_.root_ + *it;
        int fd = open(path_name.c_str(), O_RDONLY);
        if (fd == -1) {
            std::cerr << "open\n";
        } else {
            ssize_t len = read(fd, buf, 700);  // NOLINT
            response.body_ = buf;
            if (len < 0) {
                std::cerr << "read\n";
            } else {
                std::cout << buf << '\n';
            }
        }
    }
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
