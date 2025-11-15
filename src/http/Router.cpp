#include "Router.h"

#include <fcntl.h>

#include <stdexcept>
#include <vector>

#include "../core/Common.h"
#include "HttpException.h"
#include "MimeTypes.h"

Router::Router(const HttpConfig& config) : config_(config) {}

const ServerConfig* Router::find_server(const HttpRequest& request) {
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

ResolveConfig Router::resolve_config(const ServerConfig* server,  // NOLINT
                                     const LocationConfig* location) const {
    ResolveConfig resolve;
    resolve.client_max_body_size_ =
        config_.common_config_.client_max_body_size_;
    resolve.error_page_ = config_.common_config_.error_page_;
    resolve.redirect_ = config_.common_config_.redirect_;
    resolve.root_ = config_.common_config_.root_;
    resolve.index_files_ = config_.common_config_.index_files_;
    resolve.upload_store_ = config_.common_config_.upload_store_;
    resolve.autoindex_ = config_.common_config_.autoindex_;

    if (server) {
        if (server->common_config_.client_max_body_size_ > 0)
            resolve.client_max_body_size_ =
                server->common_config_.client_max_body_size_;
        if (!server->common_config_.error_page_.empty())
            resolve.error_page_ = server->common_config_.error_page_;
        if (!server->common_config_.redirect_.target.empty())
            resolve.redirect_ = server->common_config_.redirect_;
        if (!server->common_config_.root_.empty())
            resolve.root_ = server->common_config_.root_;
        if (!server->common_config_.index_files_.empty())
            resolve.index_files_ = server->common_config_.index_files_;
        if (!server->common_config_.upload_store_.empty())
            resolve.upload_store_ = server->common_config_.upload_store_;
        if (!server->listens_.address.empty())
            resolve.listens_ = server->listens_;
        if (!server->server_names_.empty())
            resolve.server_names_ = server->server_names_;
        resolve.autoindex_ = server->common_config_.autoindex_;
    }

    if (location) {
        if (location->common_config_.client_max_body_size_ > 0)
            resolve.client_max_body_size_ =
                location->common_config_.client_max_body_size_;
        if (!location->common_config_.error_page_.empty())
            resolve.error_page_ = location->common_config_.error_page_;
        if (!location->common_config_.redirect_.target.empty())
            resolve.redirect_ = location->common_config_.redirect_;
        if (!location->common_config_.root_.empty())
            resolve.root_ = location->common_config_.root_;
        if (!location->common_config_.index_files_.empty())
            resolve.index_files_ = location->common_config_.index_files_;
        if (!location->common_config_.upload_store_.empty())
            resolve.upload_store_ = location->common_config_.upload_store_;
        if (!location->allowed_methods_.empty())
            resolve.allowed_methods_ = location->allowed_methods_;
        if (!location->cgi_path_.empty())
            resolve.cgi_path_ = location->cgi_path_;
        if (!location->cgi_extension_.empty())
            resolve.cgi_extension_ = location->cgi_extension_;
        if (!location->path_.empty()) resolve.path_ = location->path_;
        resolve.autoindex_ = location->common_config_.autoindex_;
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

// static HttpResponse render_error(int status_code, const ErrorPageDirective&
// error_page) {
//     HttpResponse response;
//     response.status_code_ = status_code;
//     response.message_ = HttpStatus::reason(status_code);
//     if (std::find(error_page.statuses.begin(), error_page.statuses.end(),
//     status_code) == error_page.statuses.end())
//         // return default error page
//     }
//     // 403 エラー
// HTTP/1.1 403 Forbidden
// Server: nginx/1.29.3
// Date: Sat, 15 Nov 2025 03:49:49 GMT
// Content-Type: text/html
// Content-Length: 153
// Connection: keep-alive
// }

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

    if (response.status_code_ > 0) {
        // return render_error();
        // TODO resolve_.error_page の内容を元にレスポンスを返す
        // なければデフォルトのエラーページを返す
        // resolve_.error_page_
    }

    // 許可されてないメソッド
    if (std::find(resolve_.allowed_methods_.begin(),
                  resolve_.allowed_methods_.end(),
                  request.method_) == resolve_.allowed_methods_.end())
        // return render_error();
        throw HttpException(HttpStatus::MethodNotAllowed,
                            HttpStatus::reason(HttpStatus::MethodNotAllowed));

    std::string path_name;
    for (std::vector<std::string>::const_iterator index_files =
             resolve_.index_files_.begin();
         index_files != resolve_.index_files_.end(); ++index_files) {
        path_name = resolve_.root_ + *index_files;
        int fd = open(path_name.c_str(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("open() failed");
        }
        // std::stringだと画像データなどバイナリに対応できないため、vectorを使用する
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
    response.header_.content_type_ = MimeTypes::get_mime_type(path_name);
    return response;
}
