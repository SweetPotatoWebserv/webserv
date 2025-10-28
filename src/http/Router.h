#pragma once
#include "HttpParser.h"

// TODO HttpConfig をメンバにもつ
class Router {
   public:
    HttpResponse route(const HttpRequest& request) const;
};
