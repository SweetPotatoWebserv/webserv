#pragma once

#include "HttpResponse.h"
#include "Router.h"

class ResponseFactory {
   public:
    static HttpResponse make(const HttpRequest& request, const RouteInfo& route,
                             const HttpException& parse_error);
    static HttpResponse render_error(int status_code, const RouteInfo& route);

   private:
    static bool is_cgi(const HttpRequest& request, const RouteInfo& route);
    static HttpResponse response_cgi(const HttpRequest& request, const RouteInfo& route);
    static HttpResponse response_get(const HttpRequest& request,
                                     const RouteInfo& route);
    static HttpResponse response_post(const HttpRequest& request,
                                      const RouteInfo& route);
    static HttpResponse response_delete(const HttpRequest& request,
                                        const RouteInfo& route);
    static HttpResponse response_redirect(const RouteInfo& route);
    static HttpResponse response_autoindex(const HttpRequest& request,
                                           const RouteInfo& route);
    static HttpResponse render_default_error_page(int status_code);
    static const int DEFAULT_BUFFER_LEN = 1024;
};
