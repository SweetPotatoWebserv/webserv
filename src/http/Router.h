#pragma once
#include "../config/HttpConfig.h"
#include "HttpException.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "ResolveConfig.h"

typedef struct RouteInfo {
    const ServerConfig* server_;
    const LocationConfig* location_;
    ResolveConfig resolve_;
} RouteInfo;

class Router {
   public:
    explicit Router(const HttpConfig& config);
    RouteInfo route(const HttpRequest& request);

   private:
    const ServerConfig& find_server(const HttpRequest& request);
    static const LocationConfig& find_location(const ServerConfig& server,
                                               const std::string& path);
    HttpConfig config_;
};
