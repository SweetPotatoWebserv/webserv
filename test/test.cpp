#include <cassert>
#include <iostream>

#include "../src/cgi/handler_cgi.h"
#include "../src/core/Common.h"
#include "../src/http/HttpParser.h"

HttpRequest createTestRequest() {
    HttpRequest request;

    request.method_ = MethodPOST;

    request.request_target_.path_ = "./cgi-bin/test.py";
    request.request_target_.query_string_ = "q=search";

    request.body_ = "body_from_request!!";
    request.header_.content_length_ = request.body_.length();
    request.header_.content_type_ = "text/plain";

    return request;
}

int main() {
    CgiProcess process;
    HttpRequest test_request;
    HttpResponse test_response;

    test_request = createTestRequest();

    bool success = process.run(test_request, test_response);

    std::cout << "--- CgiProcess::run() returned ---\n";
    std::cout << "Status: " << test_response.status_code_ << "\n";
    std::cout << "Content-Type: " << test_response.header_.content_type_
              << "\n";
    std::cout << "Body:\n" << test_response.body_ << "\n";

    assert(success == true);

    assert(test_response.status_code_ == HttpStatus::OK);
    assert(test_response.header_.content_type_ == "text/plain");

    // CgiExecutorがstdin/stdoutを正しく処理したか
    assert(test_response.body_.find("Hello from CgiProcess") !=
           std::string::npos);
    assert(test_response.body_.find("body_from_request!!") !=
           std::string::npos);

    // CgiEnvBuilderがenvpを正しく構築したか
    assert(test_response.body_.find("Method: POST") != std::string::npos);
    assert(test_response.body_.find("Query String: q=search") !=
           std::string::npos);
    assert(test_response.body_.find("Content-Type: text/plain") !=
           std::string::npos);

    std::cout << "\n[SUCCESS] CgiProcess test PASSED!\n";
    return 0;
}
