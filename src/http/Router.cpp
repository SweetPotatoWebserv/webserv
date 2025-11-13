#include "Router.h"

#include <fcntl.h>

#include <stdexcept>
#include <vector>

#include "../core/Common.h"
#include "HttpException.h"

Router::Router(const HttpConfig& config) : config_(config) {}

ServerConfig* Router::find_server(const HttpRequest& request) {
    // パスを探索
    ServerConfig* matched = NULL;
    for (std::vector<ServerConfig>::iterator servers = config_.servers_.begin();
         servers != config_.servers_.end(); ++servers) {
        if (servers->listens_.port != request.host_.getPort()) continue;
        if (servers->listens_.address != DEFAULT_ADDRESS &&
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

ResolveConfig Router::resolve_config(const ServerConfig* server,
                                     const LocationConfig* location) const {
    ResolveConfig resolve;
    resolve.root_ = config_.defaults_.root_;
    resolve.index_files_ = config_.defaults_.index_files_;
    resolve.upload_store_ = config_.defaults_.upload_store_;
    resolve.client_max_body_size_ = config_.defaults_.client_max_body_size_;
    resolve.autoindex_ = config_.defaults_.autoindex_;

    if (server) {
        if (!server->common_config_.root_.empty())
            resolve.root_ = server->common_config_.root_;
        if (!server->common_config_.index_files_.empty())
            resolve.index_files_ = server->common_config_.index_files_;
        if (!server->common_config_.upload_store_.empty())
            resolve.upload_store_ = server->common_config_.upload_store_;
        if (server->common_config_.client_max_body_size_ > 0)
            resolve.client_max_body_size_ =
                server->common_config_.client_max_body_size_;
        resolve.autoindex_ = server->common_config_.autoindex_;
    }

    if (location) {
        if (!location->common_config_.root_.empty())
            resolve.root_ = location->common_config_.root_;
        if (!location->common_config_.index_files_.empty())
            resolve.index_files_ = location->common_config_.index_files_;
        if (!location->common_config_.upload_store_.empty())
            resolve.upload_store_ = location->common_config_.upload_store_;
        if (location->common_config_.client_max_body_size_ > 0)
            resolve.client_max_body_size_ =
                location->common_config_.client_max_body_size_;
        if (location->common_config_.autoindex_) resolve.autoindex_ = true;
    }
    return resolve;
}

const LocationConfig* Router::find_location(const ServerConfig& server,
                                            const HttpRequest& request) {
    const LocationConfig* location = NULL;
    for (std::vector<LocationConfig>::const_iterator locations =
             server.locations_.begin();
         locations != server.locations_.end(); ++locations) {
        if (request.request_target_.path_ == locations->path_) {
            location = &(*locations);
            break;
        }
    }
    return location;
}

HttpResponse Router::create_response(const HttpRequest& request) {  // NOLINT
    HttpResponse response;
    // 対象の server を見つける
    const ServerConfig* server = find_server(request);
    // config に server が1つもなければ、NULLになる
    // 1つでもあればマッチするものがなくても初めのサーバーがデフォルトサーバーとして設定される
    if (!server) {
        throw HttpException(HttpStatus::BadRequest,
                            HttpStatus::reason(HttpStatus::BadRequest));
    }
    const LocationConfig* location = find_location(*server, request);
    if (!location) {
        throw HttpException(HttpStatus::BadRequest,
                            HttpStatus::reason(HttpStatus::BadRequest));
    }
    resolve_ = resolve_config(server, location);

    for (std::vector<std::string>::const_iterator index_files =
             resolve_.index_files_.begin();
         index_files != resolve_.index_files_.end(); ++index_files) {
        std::string path_name = resolve_.root_ + *index_files;
        int fd = open(path_name.c_str(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("open() failed");
        }
        // std::string
        // だと画像データなどバイナリに対応できないため、vectorを使用する
        std::vector<char> buffer;
        char buf[DEFAULT_BUFFER_LEN];
        ssize_t len;
        while ((len = read(fd, buf, DEFAULT_BUFFER_LEN)) > 0) {
            buffer.insert(buffer.end(), buf, buf + len);
        }
        close(fd);
        response.body_.assign(buffer.begin(), buffer.end());
    }
    response.status_code_ = HttpStatus::OK;
    response.message_ = HttpStatus::reason(HttpStatus::OK);
    response.header_.content_length_ = response.body_.size();
    // 拡張子をみてファイルタイプを判別する
    // response.header_.content_type_ = "app";
    return response;
}
