#pragma once

#include <string>

#include "../http/HttpResponse.h"
#include "../http/ResponseFactory.h"
#include "executor_cgi.h"

class CgiProcess {
   public:
    CgiProcess();
    ~CgiProcess();
    HttpResponse run(const HttpRequest& request, const RouteInfo& info);

   private:
    CgiExecutor executor_;

    static HttpResponse validateCgiScript(const std::string& script_path);
    CgiProcess(const CgiProcess&);
    CgiProcess& operator=(const CgiProcess&);
};
