#pragma once
#include "../config/HttpConfig.h"
#include "HttpParser.h"

struct ResolveConfig {
    off_t client_max_body_size_;
    ErrorPageDirective error_page_;
    ReturnDirective redirect_;
    std::string root_;
    bool autoindex_;
    std::vector<std::string> index_files_;
    std::string upload_store_;
    std::vector<Method> allowed_methods_;
    std::string cgi_path_;
    std::string cgi_extension_;
    std::string path_;
    ListenDirective listens_;
    std::vector<std::string> server_names_;
};

class Router {
   public:
    explicit Router(const HttpConfig& config);
    HttpResponse create_response(const HttpRequest& request);

   private:
    ResolveConfig resolve_config(const ServerConfig* server,
                                 const LocationConfig* location) const;
    ServerConfig* find_server(const HttpRequest& request);
    static const LocationConfig* find_location(const ServerConfig& server,
                                               const HttpRequest& request);
    ResolveConfig resolve_;
    HttpConfig config_;
    static const int DEFAULT_BUFFER_LEN = 1024;
};
