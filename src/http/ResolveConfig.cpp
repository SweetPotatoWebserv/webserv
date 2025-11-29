#include "ResolveConfig.h"

#include <fcntl.h>

#include <stdexcept>

#include "../core/Fd.h"
#include "HttpException.h"
#include "Router.h"

ResolveConfig::ResolveConfig()
    : client_max_body_size_(CommonConfig::INVALID_NUM) {}

void ResolveConfig::resolve_error_pages_internal(  // NOLINT
    ResolveConfig& resolve, const HttpConfig& http,
    const ServerConfig& server) {
    if (resolve.error_page_.empty()) return;

    for (std::map<int, ErrorPageDirective>::iterator error_page =
             resolve.error_page_.begin();
         error_page != resolve.error_page_.end(); ++error_page) {
        ErrorPageDirective& error_page_directive = error_page->second;

        LocationConfig location;
        try {
            location =
                Router::find_location(server, error_page_directive.target);
        } catch (HttpException& e) {
            error_page_directive.target = "";
            continue;
        }

        std::string base_root;
        if (location.getCommonConfig().root_.is_set_) {
            base_root = location.getCommonConfig().root_.value_;
        } else if (server.getCommonConfig().root_.is_set_) {
            base_root = server.getCommonConfig().root_.value_;
        } else if (http.getCommonConfig().root_.is_set_) {
            base_root = http.getCommonConfig().root_.value_;
        }

        if (base_root.empty()) {
            error_page_directive.target = "";
            continue;
        }

        std::vector<std::string> indexes;

        if (!location.getCommonConfig().index_files_.empty()) {
            indexes = location.getCommonConfig().index_files_;
        } else if (!server.getCommonConfig().index_files_.empty()) {
            indexes = server.getCommonConfig().index_files_;
        } else if (!http.getCommonConfig().index_files_.empty()) {
            indexes = http.getCommonConfig().index_files_;
        }

        // ディレクトリであれば index の探索が必要 例： /error/
        // ファイル名まで指定されていれば index の探索は不要 例：/error/404.html
        if (!(error_page_directive
                  .target[error_page_directive.target.size() - 1] == '/')) {
            std::string absolute_path = base_root + error_page_directive.target;
            try {
                fd::Fd fd(absolute_path.c_str(), O_RDONLY);
                error_page_directive.target = absolute_path;
            } catch (std::runtime_error& e) {
                std::cerr << e.what() << '\n';
                error_page_directive.target = "";
            }
            continue;
        }

        bool found = false;
        for (std::vector<std::string>::iterator index = indexes.begin();
             index != indexes.end(); ++index) {
            std::string absolute_path =
                base_root + error_page_directive.target + *index;
            try {
                fd::Fd fd(absolute_path.c_str(), O_RDONLY);
                error_page_directive.target = absolute_path;
                found = true;
                break;
            } catch (std::runtime_error& e) {
                std::cerr << e.what() << '\n';
            }
        }

        if (!found) {
            error_page_directive.target = "";
        }
    }
}

ResolveConfig ResolveConfig::resolve_config(  // NOLINT
    const HttpConfig& http, const ServerConfig& server,
    const LocationConfig& location) {
    ResolveConfig resolve;

    if (http.getCommonConfig().client_max_body_size_ !=
        CommonConfig::INVALID_NUM)
        resolve.client_max_body_size_ =
            http.getCommonConfig().client_max_body_size_;
    if (http.getCommonConfig().redirect_.status != CommonConfig::INVALID_NUM)
        resolve.redirect_ = http.getCommonConfig().redirect_;
    if (http.getCommonConfig().root_.is_set_)
        resolve.root_ = http.getCommonConfig().root_;
    if (http.getCommonConfig().upload_store_.is_set_)
        resolve.upload_store_ = http.getCommonConfig().upload_store_;
    if (http.getCommonConfig().autoindex_.is_set_)
        resolve.autoindex_ = http.getCommonConfig().autoindex_;
    if (!http.getCommonConfig().error_page_.empty())
        merge_error_pages(resolve, http);
    if (!http.getCommonConfig().index_files_.empty())
        resolve.index_files_ = http.getCommonConfig().index_files_;

    if (server.getCommonConfig().client_max_body_size_ !=
        CommonConfig::INVALID_NUM)
        resolve.client_max_body_size_ =
            server.getCommonConfig().client_max_body_size_;
    if (server.getCommonConfig().redirect_.status != CommonConfig::INVALID_NUM)
        resolve.redirect_ = server.getCommonConfig().redirect_;
    if (server.getCommonConfig().root_.is_set_)
        resolve.root_ = server.getCommonConfig().root_;
    if (server.getCommonConfig().upload_store_.is_set_)
        resolve.upload_store_ = server.getCommonConfig().upload_store_;
    if (server.getCommonConfig().autoindex_.is_set_)
        resolve.autoindex_ = server.getCommonConfig().autoindex_;
    if (!server.getCommonConfig().error_page_.empty())
        merge_error_pages(resolve, server);
    if (!server.getCommonConfig().index_files_.empty())
        resolve.index_files_ = server.getCommonConfig().index_files_;
    if (!server.getListens().address.empty())
        resolve.listens_ = server.getListens();
    if (!server.getServerNames().empty())
        resolve.server_names_ = server.getServerNames();

    if (location.getCommonConfig().client_max_body_size_ !=
        CommonConfig::INVALID_NUM)
        resolve.client_max_body_size_ =
            location.getCommonConfig().client_max_body_size_;
    if (location.getCommonConfig().redirect_.status !=
        CommonConfig::INVALID_NUM)
        resolve.redirect_ = location.getCommonConfig().redirect_;
    if (location.getCommonConfig().root_.is_set_)
        resolve.root_ = location.getCommonConfig().root_;
    if (location.getCommonConfig().upload_store_.is_set_)
        resolve.upload_store_ = location.getCommonConfig().upload_store_;
    if (location.getCommonConfig().autoindex_.is_set_)
        resolve.autoindex_ = location.getCommonConfig().autoindex_;
    if (!location.getCommonConfig().error_page_.empty())
        merge_error_pages(resolve, location);
    if (!location.getCommonConfig().index_files_.empty())
        resolve.index_files_ = location.getCommonConfig().index_files_;
    if (!location.getAllowedMethods().empty())
        resolve.allowed_methods_ = location.getAllowedMethods();
    if (!location.getCgiPath().empty())
        resolve.cgi_path_ = location.getCgiPath();
    if (!location.getCgiExtension().empty())
        resolve.cgi_extension_ = location.getCgiExtension();
    if (!location.getPath().empty()) resolve.path_ = location.getPath();

    ResolveConfig::resolve_error_pages_internal(resolve, http, server);
    return resolve;
}
