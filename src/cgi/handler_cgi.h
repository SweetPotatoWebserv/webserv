#pragma once

#include <string>

#include "../http/HttpResponse.h"
#include "../http/ResponseFactory.h"
#include "executor_cgi.h"
#include "response_cgi.h"

struct CgiSession {
    pid_t pid;
    int readFd;   // ここからCGIの出力を読む (EPOLLIN)
    int writeFd;  // ここへBodyを書き込む (EPOLLOUT)

    std::string bodyBuffer;  // 送信待ちのリクエストボディ
    size_t sentBytes;        // 送信済みのバイト数

    std::time_t startTime;
    std::string responseBuffer;  // 受信したレスポンスを溜める場所

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
