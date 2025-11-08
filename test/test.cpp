#include <cassert>
#include <iostream>

#include "../src/cgi/handler_cgi.h"
#include "../src/core/Common.h"
#include "../src/http/HttpParser.h"

int main() {
    CgiProcess process;
    HttpRequest test_request;
    HttpResponse test_response;

    test_request.method_ = MethodPOST;
    test_request.header_.body_ = "body_from_request!!";
    test_request.header_.content_length_ = test_request.header_.body_.length();
    test_request.header_.content_type_ = "text/plain";

    test_request.request_target_.path_ = "../cgi-bin/test.py";
    test_request.request_target_.query_string_ = "q=search";

    bool success = process.run(test_request, test_response);

    assert(success == true);
    std::cout << "CgiProcess::run() returned body:\n";
    std::cout << test_response.header_.body_;

    //    assert(test_response.header_.body_.find("Hello from CgiProcess") !=
    //           std::string::npos);
    assert(test_response.header_.body_.find("body_from_request!!") !=
           std::string::npos);

    assert(test_response.status_code_ == 200);

    std::cout << "\n[SUCCESS] CgiProcess test PASSED!\n";
    return 0;
}
