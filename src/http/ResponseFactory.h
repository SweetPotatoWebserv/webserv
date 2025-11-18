#pragma once

#include "HttpResponse.h"
#include "Router.h"

class ResponseFactory {
   public:
    static HttpResponse make(const HttpRequest& request, const RouteInfo& route,
                             const HttpException& parse_error);

   private:
    static HttpResponse response_get(const HttpRequest& request,
                                     const RouteInfo& route);
    static HttpResponse response_post(const HttpRequest& request,
                                      const RouteInfo& route);
    static HttpResponse response_delete(const HttpRequest& request,
                                        const RouteInfo& route);
    static HttpResponse response_redirect(const RouteInfo& route);
    static HttpResponse response_autoindex(const HttpRequest& request,
                                           const RouteInfo& route);
    static const int DEFAULT_BUFFER_LEN = 1024;
};
