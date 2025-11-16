#pragma once
#include "../config/HttpConfig.h"
#include "HttpParser.h"
#include "ResolveConfig.h"

class Router {
   public:
    explicit Router(const HttpConfig& config);
    HttpResponse create_response(const HttpRequest& request);
    static const LocationConfig& find_location(const ServerConfig& server,
                                               const std::string& path);

   private:
    HttpResponse render_error(int status_code,
                              const std::string& error_page = "");
    const ServerConfig& find_server(const HttpRequest& request);
    ResolveConfig resolve_;
    HttpConfig config_;
    static const int DEFAULT_BUFFER_LEN = 1024;
};
