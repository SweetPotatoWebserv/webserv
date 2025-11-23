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
    // bool is_executable = ((st.st_mode & S_IXUSR) != 0) ||
    //                      ((st.st_mode & S_IXGRP) != 0) ||
    //                      ((st.st_mode & S_IXOTH) != 0);

    // bool is_readable = ((st.st_mode & S_IRUSR) != 0) ||
    //                    ((st.st_mode & S_IRGRP) != 0) ||
    //                    ((st.st_mode & S_IROTH) != 0);

    // if (!is_executable || !is_readable) {
    //     response.status_code_ = HttpStatus::Forbidden;
    //     response.body_ = "CGI script is not executable or readable";
    //     return response;
    // }

    std::cout << script_path << "\n";
    if (access(script_path.c_str(), X_OK) != 0) {
        std::cout << "you cant execute";
    }

    return response;
}

HttpResponse CgiProcess::run(const HttpRequest& request,
                             const RouteInfo& info) {
    char** argv = NULL;
    char** envp = NULL;
    HttpResponse response;
    try {
        // const std::string& script_path =
        //     info.resolve_.root_.value_ + request.request_target_.path_;
        // Check if strings are empty to avoid segfaults on index access
        if (base.empty()) {
            script_path = req_path;
        } else if (req_path.empty()) {
            script_path = base;
        } else {
            // Check for slashes
            bool base_ends_slash = (base[base.length() - 1] == '/');
            bool req_starts_slash = (req_path[0] == '/');

            if (base_ends_slash && req_starts_slash) {
                // Case 1: Both have slashes (e.g., "/var/www/" + "/index.html")
                // Remove one slash to avoid "//"
                script_path = base + req_path.substr(1);
            } else if (!base_ends_slash && !req_starts_slash) {
                // Case 2: Neither has a slash (e.g., "/var/www" + "index.html")
                // Add a slash
                script_path = base + "/" + req_path;
            } else {
                // Case 3: Perfectly formatted (one has slash, one doesn't)
                // Just concatenate
                script_path = base + req_path;
            }
        }

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
