#pragma once

#include <string>

#include "../http/HttpResponse.h"
#include "../http/ResponseFactory.h"
#include "executor_cgi.h"

struct CgiSession {
    pid_t pid;
    int readFd;
    int writeFd;

    std::string bodyBuffer;
    size_t sentBytes;

    std::time_t startTime;
    std::string responseBuffer;

    CgiSession()
        : pid(-1), readFd(-1), writeFd(-1), sentBytes(0), startTime(0) {}
};

class CgiProcess {
   public:
    CgiProcess();
    ~CgiProcess();
    static void parseCgiResponse(HttpResponse& response,
                                 const std::string& raw_output);
    CgiSession startCgi(const HttpRequest& request, const RouteInfo& info);

   private:
    CgiExecutor executor_;

    static HttpResponse validateCgiScript(const std::string& script_path);
    CgiProcess(const CgiProcess&);
    CgiProcess& operator=(const CgiProcess&);
};
