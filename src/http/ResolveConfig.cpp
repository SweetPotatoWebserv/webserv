#include "ResolveConfig.h"

ResolveConfig::ResolveConfig()
    : client_max_body_size_(CommonConfig::INVALID_NUM) {}

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
        resolve.error_page_ = http.getCommonConfig().error_page_;
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
        resolve.error_page_ = server.getCommonConfig().error_page_;
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
        resolve.error_page_ = location.getCommonConfig().error_page_;
    if (!location.getCommonConfig().index_files_.empty())
        resolve.index_files_ = location.getCommonConfig().index_files_;
    if (!location.getAllowedMethods().empty())
        resolve.allowed_methods_ = location.getAllowedMethods();
    if (!location.getCgiPath().empty())
        resolve.cgi_path_ = location.getCgiPath();
    if (!location.getCgiExtension().empty())
        resolve.cgi_extension_ = location.getCgiExtension();
    if (!location.getPath().empty()) resolve.path_ = location.getPath();

    return resolve;
}
