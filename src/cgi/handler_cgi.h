#pragma once

#include <string>

#include "../http/HttpParser.h"
#include "executor_cgi.h"

class CgiProcess {
   public:
    bool run(const HttpRequest& request, HttpResponse& response);

   private:
    CgiProcess();
    ~CgiProcess();

    CgiExecutor executor_;

    static bool validateCgiScript(const std::string& script_path,
                                  HttpResponse& response);
    CgiProcess(const CgiProcess&);
    CgiProcess& operator=(const CgiProcess&);
};
