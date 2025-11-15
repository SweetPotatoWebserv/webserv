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

CgiProcess::CgiProcess() { executor_ = new CgiExecutor(); }

CgiProcess::~CgiProcess() { delete executor_; }

bool CgiProcess::run(const HttpRequest& request, HttpResponse& response) {
    char** argv = NULL;
    char** envp = NULL;
    try {
        std::string script_path = request.request_target_.path_;
        struct stat st;

        if (stat(script_path.c_str(), &st) == -1) {
            response.status_code_ = HttpStatus::NotFound;
            response.body_ = "CGI script not found.";
            return true;
        }

        if (!S_ISREG(st.st_mode)) {
            response.status_code_ = HttpStatus::Forbidden;
            response.body_ = "CGI target is not a regular file.";
            return true;
        }

        bool is_executable = ((st.st_mode & S_IXUSR) != 0) ||
                             ((st.st_mode & S_IXGRP) != 0) ||
                             ((st.st_mode & S_IXOTH) != 0);
        if (!is_executable) {
            response.status_code_ = HttpStatus::Forbidden;
            response.body_ = "CGI script is not executable (no 'x' bit set)";
            return true;
        }

        argv = createArgv(script_path);
        envp = createEnvp(request);

        const std::string& request_body = request.body_;

        std::string raw_output;
        try {
            raw_output =
                executor_->execute(script_path, argv, envp, request_body);
        } catch (const CgiExecutionException& e) {
            // CgiExecutor(fork, pipe, execve, waitpid)でエラーが発生した場合
            std::cerr << "CGI Execution failed: " << e.what() << "\n";

            response.status_code_ = e.getStatusCode();
            response.body_ = e.what();

            freeArray(argv);
            freeArray(envp);
            return true;
        }

        parseCgiResponse(response, raw_output);

        freeArray(argv);
        freeArray(envp);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "CgiProcess FATAL error: " << e.what() << "\n";
        freeArray(argv);
        freeArray(envp);

        // エラーページすら生成できなかった時のみエラーを返す
        return false;
    }
}
