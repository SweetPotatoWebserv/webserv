#pragma once

#include "../config/HttpConfig.h"

struct ResolveConfig {
    ResolveConfig();
    off_t client_max_body_size_;
    std::map<int, ErrorPageDirective> error_page_;
    ReturnDirective redirect_;
    RootDirective root_;
    std::vector<std::string> index_files_;
    UploadStoreDirective upload_store_;
    std::vector<Method> allowed_methods_;
    std::string cgi_path_;
    std::string cgi_extension_;
    std::string path_;
    ListenDirective listens_;
    std::vector<std::string> server_names_;
    AutoIndexDirective autoindex_;
    static ResolveConfig resolve_config(const HttpConfig& http,
                                        const ServerConfig& server,
                                        const LocationConfig& location);
};
