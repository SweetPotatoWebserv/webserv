#pragma once
#include "HttpParser.h"

// TODO HttpConfig をメンバにもつ
// レスポンスを作る
class Router {
   public:
    HttpResponse route(const HttpRequest& request) const;
};
