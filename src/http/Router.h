#pragma once
#include "../config/HttpConfig.h"
#include "HttpException.h"
#include "HttpParser.h"
#include "ResolveConfig.h"

typedef struct RouteInfo {
    const ServerConfig* server_;
    const LocationConfig* location_;
    ResolveConfig resolve_;
} RouteInfo;

class Router {
   public:
    explicit Router(const HttpConfig& config);
    RouteInfo route(const HttpRequest& request) const;
    static const LocationConfig& find_location(const ServerConfig& server,
                                               const std::string& path);

   private:
    const ServerConfig& find_server(const HttpRequest& request) const;
    HttpConfig config_;
};
