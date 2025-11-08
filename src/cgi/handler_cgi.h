#pragma once

#include <string>

#include "../http/HttpParser.h"
#include "executor_cgi.h"

class CgiProcess {
   public:
    CgiProcess();
    ~CgiProcess();

    bool run(const HttpRequest& request, HttpResponse& response);

   private:
    CgiExecutor* executor_;

    CgiProcess(const CgiProcess&);
    CgiProcess& operator=(const CgiProcess&);
};

class CgiEnvBuilder {
   public:
    // void setEnv();
    // void deleteEnv();
};
