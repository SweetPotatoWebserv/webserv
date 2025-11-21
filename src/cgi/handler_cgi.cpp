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
#include "../http/ResponseFactory.h"

CgiProcess::CgiProcess() {}

CgiProcess::~CgiProcess() {}

HttpResponse CgiProcess::validateCgiScript(const std::string& script_path) {
    HttpResponse response;
    struct stat st;

    response.status_code_ = HttpStatus::OK;
    std::cout << script_path << '\n';
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

    // ファイルのパーミッション(st_mode)から、
    // ユーザー(S_IXUSR)、グループ(S_IXGRP)、その他(S_IXOTH)の
    // いずれかに実行権限('x'ビット)があるかを確認する。
    bool is_executable = ((st.st_mode & S_IXUSR) != 0) ||
                         ((st.st_mode & S_IXGRP) != 0) ||
                         ((st.st_mode & S_IXOTH) != 0);
    if (!is_executable) {
        response.status_code_ = HttpStatus::Forbidden;
        response.body_ = "CGI script is not executable (no 'x' bit set)";
        return response;
    }

    return response;
}

HttpResponse CgiProcess::run(const HttpRequest& request, const RouteInfo& route) {
    char** argv = NULL;
    char** envp = NULL;
    HttpResponse response;
    try {
        if (!route.resolve_.root_.is_set_)
            return ResponseFactory::render_error(HttpStatus::NotFound, route);
        const std::string& script_path = route.resolve_.root_.value_ + request.request_target_.path_;

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
            freeArray(argv);
            freeArray(envp);
            return response;
        }

        parseCgiResponse(response, raw_output);
        freeArray(argv);
        freeArray(envp);
        return response;
    } catch (const std::exception& e) {
        std::cerr << "CgiProcess FATAL error: " << e.what() << "\n";
        freeArray(argv);
        freeArray(envp);

        // エラーページすら生成できなかった時のみエラーを返す
        return response;
    }
}
