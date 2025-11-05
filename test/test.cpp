#include <cassert>
#include <iostream>

#include "../src/cgi/handler_cgi.h"

int main() {
    // 1. 準備
    CgiProcess process;
    HttpRequest dummy_request;  // "body_from_request!!" を返すダミー
    HttpResponse dummy_response;

    // 2. 実行
    bool success = process.run(dummy_response);

    // 3. 検証
    assert(success == true);
    std::cout << "CgiProcess::run() returned:\n";
    std::cout << dummy_response.getBody();

    // post_test.py の出力が含まれているか確認
    assert(dummy_response.getBody().find("Hello from CgiProcess") !=
           std::string::npos);
    assert(dummy_response.getBody().find("body_from_request!!") !=
           std::string::npos);

    std::cout << "\n[SUCCESS] CgiProcess test PASSED!\n";
    return 0;
}
