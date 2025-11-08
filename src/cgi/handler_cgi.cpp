#include "handler_cgi.h"

#include <cstring>
#include <iostream>

#include "../http/HttpParser.h"

char** createDummyArgv(const std::string& script_path) {
    char** argv = new char*[2];
    argv[0] = strdup(script_path.c_str());
    argv[1] = NULL;
    return argv;
}

/*char** createDummyEnvp() {
    char** envp = new char*[1];
    envp[0] = NULL;
    return envp;
}*/

void freeArray(char** arr) {
    if (!arr) return;
    for (int i = 0; arr[i] != NULL; ++i) {
        free(arr[i]);
    }
    delete[] arr;
}

CgiProcess::CgiProcess() {
    // _reqeuest = new CgiRequestParser();
    executor_ = new CgiExecutor();
    // _envBuilder = new CgiEnvBuilder();
    // _response = new CgiResponseParser();
}

CgiProcess::~CgiProcess() {
    // delete _reqeuest;
    delete executor_;
    // delete _envBuilder;
    // delete _response;
}

bool CgiProcess::run(HttpResponse& response) {
    // ( envBuilder->build(request, ...) )

    // --- 現時点でのダミー設定 ---
    std::string script_path =
        "../cgi-bin/test.py";  // stdinをエコーするスクリプト
    char** argv = createDummyArgv(script_path);
    char** envp = createDummyEnvp();
    std::string request_body = HttpRequest::body();
    // --- ダミーここまで ---

    try {
        // 2. CgiExecutor で実行
        std::string raw_output =
            executor_->execute(script_path, argv, envp, request_body);

        // ( parser->parse(raw_output, response) )

        // --- 現時点でのダミー処理 ---

        response.setBody(raw_output);
        // --- ダミーここまで ---

    } catch (const CgiExecutionException& e) {
        std::cerr
            << "CGI failed: " << e.what()
            << " (Status: 実装してないよ\n";  //<< e.getStatusCode() << ")\n";
        // ( response->setError(e.getStatusCode()) )
        freeArray(argv);
        freeArray(envp);
        return false;
    }

    // 4. クリーンアップ
    freeArray(argv);
    freeArray(envp);
    return true;
}
