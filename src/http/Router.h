#pragma once
#include "../config/HttpConfig.h"
#include "HttpParser.h"

class Router {
   public:
    explicit Router(const HttpConfig& config);
    HttpResponse create_response(const HttpRequest& request);
    ServerConfig* find_server(const HttpRequest& request);

   private:
    HttpConfig config_;
};
