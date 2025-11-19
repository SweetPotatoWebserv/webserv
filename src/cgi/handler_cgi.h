#pragma once

#include <string>

#include "../http/HttpResponse.h"
#include "executor_cgi.h"

class CgiProcess {
   public:
    CgiProcess();
    ~CgiProcess();
    bool run(const HttpRequest& request, HttpResponse& response);

   private:
    CgiExecutor executor_;

    static bool validateCgiScript(const std::string& script_path,
                                  HttpResponse& response);
    CgiProcess(const CgiProcess&);
    CgiProcess& operator=(const CgiProcess&);
};
