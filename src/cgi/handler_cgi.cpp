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
#include "response_cgi.h"

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

HttpResponse CgiProcess::run(const HttpRequest& request,
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

        response = validateCgiScript(script_path);
        if (response.status_code_ != HttpStatus::OK) {
            return response;
        }

        argv = createArgv(script_path);
        envp = createEnvp(request);

        const std::string& request_body = request.body_;

        std::string raw_output;
        try {
            raw_output =
                executor_.execute(script_path, argv, envp, request_body);
        } catch (const CgiExecutionException& e) {
            // CgiExecutor(fork, pipe, execve, waitpid)でエラーが発生した場合
            std::cerr << "CGI Execution failed: " << e.what() << "\n";

            response.status_code_ = e.getStatusCode();
            response.body_ = e.what();
            deleteArray(argv);
            deleteArray(envp);
            return response;
        }

        parseCgiResponse(response, raw_output);
        deleteArray(argv);
        deleteArray(envp);
        return response;
    } catch (const std::exception& e) {
        std::cerr << "CgiProcess FATAL error: " << e.what() << "\n";
        deleteArray(argv);
        deleteArray(envp);

        return response;
    }
}
