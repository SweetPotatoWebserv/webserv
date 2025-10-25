#pragma once
#include "HttpParser.h"

class Router {
   public:
    HttpResponse route(const HttpRequest& request) const;
};
