#include "handler_cgi.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "../core/Common.h"
#include "../http/HttpParser.h"
#include "executor_cgi.h"
#include "request_cgi.h"

CgiProcess::CgiProcess() {}

CgiProcess::~CgiProcess() {}

HttpResponse CgiProcess::validateCgiScript(const std::string& script_path) {
    HttpResponse response;
    struct stat st;

    response.status_code_ = HttpStatus::OK;
    if (stat(script_path.c_str(), &st) == -1) {
        response.status_code_ = HttpStatus::NotFound;
        response.body_ = "CGI script not found.";
        return response;
    }

    if (!S_ISREG(st.st_mode)) {
        response.status_code_ = HttpStatus::Forbidden;
        response.body_ = "CGI target is not a regular file.";
        return response;
    }

    if (access(script_path.c_str(), X_OK) != 0 ||
        access(script_path.c_str(), R_OK) != 0) {
        response.status_code_ = HttpStatus::Forbidden;
        response.body_ = "CGI script is not executable or readable";
        return response;
    }

    return response;
}

CgiSession CgiProcess::startCgi(const HttpRequest& request,
                                const RouteInfo& info) {
    char** argv = NULL;
    char** envp = NULL;
    HttpResponse response;
    try {
        std::string path_str = request.request_target_.path_;

        if (!path_str.empty() && path_str[0] == '/') {
            path_str = path_str.substr(1);
        }

        std::string script_path = info.resolve_.root_.value_ + path_str;

        CgiSession session;
        response = validateCgiScript(script_path);
        if (response.status_code_ != HttpStatus::OK) {
            throw HttpException(response.status_code_);
        }
        argv = createArgv(script_path);
        envp = createEnvp(request);

        CgiResult result = executor_.execute(script_path, argv, envp);

        session.pid = result.pid;
        session.readFd = result.readFd;
        session.writeFd = result.writeFd;
        session.bodyBuffer = request.body_;
        session.sentBytes = 0;
        session.startTime = std::time(NULL);
        deleteArray(argv);
        deleteArray(envp);
        return session;
    } catch (const std::exception& e) {
        std::cerr << "CGI Unexpected Error: " << e.what() << "\n";
        deleteArray(argv);
        deleteArray(envp);
        throw HttpException(HttpStatus::InternalServerError);
    }
}
