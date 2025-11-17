#pragma once
#include "HttpParser.h"
#include "HttpResponse.h"

// TODO HttpConfig をメンバにもつ
class Router {
   public:
    HttpResponse route(const HttpRequest& request) const;
};
