#include "handler_cgi.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
    try {
        std::string script_path = request.request_target_.path_;
        if (access(script_path.c_str(), F_OK) == -1) {
            response.status_code_ = HttpStatus::NotFound;
            response.body_ = "CGI script not found.";
            return true;
        }

        if (access(script_path.c_str(), X_OK) == -1) {
            response.status_code_ = HttpStatus::Forbidden;
            response.body_ = "CGI script is not executable.";
            return true;
        }
        // CgiEnvBuilderなどに移行したい(envpにしてexecveの時に渡せば他の環境変数は引き継がれないので削除する必要がない)
        char** argv = createArgv(script_path);
        char** envp = createEnvp(request);

        // HttpRequestからstdin(リクエストボディ)の内容を取得
        const std::string& request_body = request.body_;

        std::string raw_output;
        try {
            // CgiExecutorでスクリプトを実行
            raw_output =
                executor_->execute(script_path, argv, envp, request_body);
        } catch (const CgiExecutionException& e) {
            // CgiExecutor(fork, pipe, execve, waitpid)でエラーが発生した場合
            std::cerr << "CGI Execution failed: " << e.what() << "\n";

            // エラーレスポンスを生成
            response.status_code_ = e.getStatusCode();
            response.body_ = e.what();

            freeArray(argv);
            freeArray(envp);
            return true;
        }

        // CGIの生出力(stdoutから親proccessが読み取ったもの)をパースしてHttpResponseに格納
        parseCgiResponse(response, raw_output);

        freeArray(argv);
        freeArray(envp);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "CgiProcess FATAL error: " << e.what() << "\n";

        // エラーページすら生成できなかった、という意味でfalseを返す
        return false;
    }
}
