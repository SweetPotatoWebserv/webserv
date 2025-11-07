#pragma once
#include "../config/HttpConfig.h"
#include "HttpParser.h"

// TODO HttpConfig をメンバにもつ
// レスポンスを作る
class Router {
   public:
    explicit Router(const HttpConfig& config);
    HttpResponse create_response(const HttpRequest& request) const;

   private:
    HttpConfig config_;
};
