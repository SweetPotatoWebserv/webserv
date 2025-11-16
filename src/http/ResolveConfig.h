#pragma once

#include "../config/HttpConfig.h"

struct ResolveConfig {
    off_t client_max_body_size_;
    std::map<int, ErrorPageDirective> error_page_;
    ReturnDirective redirect_;
    std::string root_;
    std::vector<std::string> index_files_;
    std::string upload_store_;
    std::vector<Method> allowed_methods_;
    std::string cgi_path_;
    std::string cgi_extension_;
    std::string path_;
    ListenDirective listens_;
    std::vector<std::string> server_names_;
    bool autoindex_;
    static ResolveConfig resolve_config(const HttpConfig& http,
                                        const ServerConfig& server,
                                        const LocationConfig& location);
};
