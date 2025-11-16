#pragma once
#include "../config/HttpConfig.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "ResolveConfig.h"

class Router {
   public:
    explicit Router(const HttpConfig& config);
    HttpResponse create_response(const HttpRequest& request);
    static const LocationConfig& find_location(const ServerConfig& server,
                                               const std::string& path);

   private:
    const ServerConfig& find_server(const HttpRequest& request);
    ResolveConfig resolve_;
    HttpConfig config_;
    static const int DEFAULT_BUFFER_LEN = 1024;
};
