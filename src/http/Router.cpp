#include "Router.h"

#include <fcntl.h>
#include <sys/fcntl.h>

#include <stdexcept>
#include <vector>

#include "../core/Common.h"
#include "../http/HttpParser.h"
#include "HttpException.h"
#include "HttpParser.h"
#include "MimeTypes.h"
#include "ResolveConfig.h"

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

const LocationConfig& Router::find_location(const ServerConfig& server,
                                            const std::string& path) {
    const LocationConfig* location = NULL;
    for (std::vector<LocationConfig>::const_iterator locations =
             server.getLocations().begin();
         locations != server.getLocations().end(); ++locations) {
        if (path == locations->getPath()) {
            location = &(*locations);
            break;
        }
    }
    if (!location)
        throw HttpException(HttpStatus::NotFound,
                            HttpStatus::reason(HttpStatus::NotFound));
    return *location;
}

HttpResponse Router::create_response(const HttpRequest& request) {  // NOLINT
    HttpResponse response;
    // 対象の server を見つける
    const ServerConfig& server = find_server(request);
    // config に server が1つもなければ、NULLになる
    // 1つでもあればマッチするものがなくても初めのサーバーがデフォルトサーバーとして設定される
    try {
        const LocationConfig& location =
            find_location(server, request.request_target_.path_);
        resolve_ = ResolveConfig::resolve_config(config_, server, location);
    } catch (HttpException& e) {
        // render_error(e.status_code());
        std::cerr << e.what() << '\n';
    }

    // if (response.status_code_ > 0) {
    //     return HttpResponse::render_error(response.status_code_,
    //     resolve_.error_page_, server);
    // }
    //
    // // 許可されてないメソッド
    // if (std::find(resolve_.allowed_methods_.begin(),
    //               resolve_.allowed_methods_.end(),
    //               request.method_) == resolve_.allowed_methods_.end())
    //     return HttpResponse::render_error(HttpStatus::MethodNotAllowed,
    //     resolve_.error_page_, server);

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
