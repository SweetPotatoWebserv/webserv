#include "Router.h"

#include <fcntl.h>

#include <stdexcept>
#include <vector>

#include "../core/Common.h"
#include "HttpException.h"
#include "MimeTypes.h"

Router::Router(const HttpConfig& config) : config_(config) {}

const ServerConfig& Router::find_server(const HttpRequest& request) {
    // パスを探索
    const ServerConfig* matched = NULL;
    const std::vector<ServerConfig>& servers = config_.getservers();
    for (std::vector<ServerConfig>::const_iterator servers_itr =
             servers.begin();
         servers_itr != servers.end(); ++servers_itr) {
        if (servers_itr->getListens().port != request.host_.getPort()) continue;
        if (servers_itr->getListens().address != DEFAULT_ADDRESS &&
            servers_itr->getListens().address != request.host_.getAddress())
            continue;
        for (std::vector<std::string>::const_iterator server_names =
                 servers_itr->getServerNames().begin();
             server_names != servers_itr->getServerNames().end();
             ++server_names) {
            if (*server_names == request.host_.getAddress()) {
                matched = &(*servers_itr);
                break;
            }
        }
        if (matched) break;
    }
    if (!matched && !servers.empty()) matched = &servers.front();
    return *matched;
}

ResolveConfig Router::resolve_config(const ServerConfig& server,  // NOLINT
                                     const LocationConfig& location) const {
    ResolveConfig resolve;
    resolve.client_max_body_size_ =
        config_.getCommonConfig().client_max_body_size_;
    resolve.error_page_ = config_.getCommonConfig().error_page_;
    resolve.redirect_ = config_.getCommonConfig().redirect_;
    resolve.root_ = config_.getCommonConfig().root_;
    resolve.index_files_ = config_.getCommonConfig().index_files_;
    resolve.upload_store_ = config_.getCommonConfig().upload_store_;
    resolve.autoindex_ = config_.getCommonConfig().autoindex_;

    if (server.getCommonConfig().client_max_body_size_ > 0)
        resolve.client_max_body_size_ =
            server.getCommonConfig().client_max_body_size_;
    if (!server.getCommonConfig().error_page_.empty())
        resolve.error_page_ = server.getCommonConfig().error_page_;
    if (!server.getCommonConfig().redirect_.target.empty())
        resolve.redirect_ = server.getCommonConfig().redirect_;
    if (!server.getCommonConfig().root_.empty())
        resolve.root_ = server.getCommonConfig().root_;
    if (!server.getCommonConfig().index_files_.empty())
        resolve.index_files_ = server.getCommonConfig().index_files_;
    if (!server.getCommonConfig().upload_store_.empty())
        resolve.upload_store_ = server.getCommonConfig().upload_store_;
    if (!server.getListens().address.empty())
        resolve.listens_ = server.getListens();
    if (!server.getServerNames().empty())
        resolve.server_names_ = server.getServerNames();
    resolve.autoindex_ = server.getCommonConfig().autoindex_;

    if (location.getCommonConfig().client_max_body_size_ > 0)
        resolve.client_max_body_size_ =
            location.getCommonConfig().client_max_body_size_;
    if (!location.getCommonConfig().error_page_.empty())
        resolve.error_page_ = location.getCommonConfig().error_page_;
    if (!location.getCommonConfig().redirect_.target.empty())
        resolve.redirect_ = location.getCommonConfig().redirect_;
    if (!location.getCommonConfig().root_.empty())
        resolve.root_ = location.getCommonConfig().root_;
    if (!location.getCommonConfig().index_files_.empty())
        resolve.index_files_ = location.getCommonConfig().index_files_;
    if (!location.getCommonConfig().upload_store_.empty())
        resolve.upload_store_ = location.getCommonConfig().upload_store_;
    if (!location.getAllowdMethods().empty())
        resolve.allowed_methods_ = location.getAllowdMethods();
    if (!location.getCgiPath().empty())
        resolve.cgi_path_ = location.getCgiPath();
    if (!location.getCgiExtension().empty())
        resolve.cgi_extension_ = location.getCgiExtension();
    if (!location.getPath().empty()) resolve.path_ = location.getPath();
    resolve.autoindex_ = location.getCommonConfig().autoindex_;
    return resolve;
}

const LocationConfig& Router::find_location(const ServerConfig& server,
                                            const HttpRequest& request) {
    const LocationConfig* location = NULL;
    for (std::vector<LocationConfig>::const_iterator locations =
             server.getLocations().begin();
         locations != server.getLocations().end(); ++locations) {
        if (request.request_target_.path_ == locations->getPath()) {
            location = &(*locations);
            break;
        }
    }
    if (!location)
        throw HttpException(HttpStatus::NotFound,
                            HttpStatus::reason(HttpStatus::NotFound));
    return *location;
}

// static HttpResponse render_error(int status_code, const ErrorPageDirective&
// error_page) {
//     HttpResponse response;
//     response.status_code_ = status_code;
//     response.message_ = HttpStatus::reason(status_code);
//     if (std::find(error_page.statuses.begin(), error_page.statuses.end(),
//     status_code) == error_page.statuses.end())
// return default error page
// }
// 403 エラー
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
    const ServerConfig& server = find_server(request);
    // config に server が1つもなければ、NULLになる
    // 1つでもあればマッチするものがなくても初めのサーバーがデフォルトサーバーとして設定される
    try {
        const LocationConfig& location = find_location(server, request);
        resolve_ = resolve_config(server, location);
    } catch (HttpException& e) {
        // render_error(e.status_code());
        std::cerr << e.what() << '\n';
    }

    // if (response.status_code_ > 0) {
    //     // return render_error();
    //     // TODO resolve_.error_page の内容を元にレスポンスを返す
    //     // なければデフォルトのエラーページを返す
    //     // resolve_.error_page_
    // }

    // 許可されてないメソッド
    // if (std::find(resolve_.allowed_methods_.begin(),
    //               resolve_.allowed_methods_.end(),
    //               request.method_) == resolve_.allowed_methods_.end())
    //     // return render_error();
    //     throw HttpException(HttpStatus::MethodNotAllowed,
    //                         HttpStatus::reason(HttpStatus::MethodNotAllowed));

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
