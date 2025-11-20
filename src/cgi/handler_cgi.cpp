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

HttpResponse CgiProcess::run(const HttpRequest& request) {
    char** argv = NULL;
    char** envp = NULL;
    HttpResponse response;
    try {
        const std::string& script_path = "/src" + request.request_target_.path_;

        response = validateCgiScript(script_path);
        if (response.status_code_ != HttpStatus::OK) {
            response.header_.content_length_ = response.body_.size();
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
            response.header_.content_length_ = response.body_.size();
            freeArray(argv);
            freeArray(envp);
            return response;
        }

        parseCgiResponse(response, raw_output);
        response.header_.content_length_ = response.body_.size();
        freeArray(argv);
        freeArray(envp);
        return response;
    } catch (const std::exception& e) {
        std::cerr << "CgiProcess FATAL error: " << e.what() << "\n";
        freeArray(argv);
        freeArray(envp);

        response.header_.content_length_ = response.body_.size();
        // エラーページすら生成できなかった時のみエラーを返す
        return response;
    }
}
