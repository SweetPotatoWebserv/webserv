#include "ResolveConfig.h"

ResolveConfig ResolveConfig::resolve_config(
    const HttpConfig& http, const ServerConfig& server,  // NOLINT
    const LocationConfig& location) {
    ResolveConfig resolve;
    resolve.client_max_body_size_ =
        http.getCommonConfig().client_max_body_size_;
    resolve.error_page_ = http.getCommonConfig().error_page_;
    resolve.redirect_ = http.getCommonConfig().redirect_;
    resolve.root_ = http.getCommonConfig().root_;
    resolve.index_files_ = http.getCommonConfig().index_files_;
    resolve.upload_store_ = http.getCommonConfig().upload_store_;
    resolve.autoindex_ = http.getCommonConfig().autoindex_;

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
    if (!location.getAllowedMethods().empty())
        resolve.allowed_methods_ = location.getAllowedMethods();
    if (!location.getCgiPath().empty())
        resolve.cgi_path_ = location.getCgiPath();
    if (!location.getCgiExtension().empty())
        resolve.cgi_extension_ = location.getCgiExtension();
    if (!location.getPath().empty()) resolve.path_ = location.getPath();
    resolve.autoindex_ = location.getCommonConfig().autoindex_;
    return resolve;
}
