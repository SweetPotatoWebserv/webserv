#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../src/cgi/handler_cgi.h"
#include "../src/core/Common.h"
#include "../src/http/HttpParser.h"
#include "../src/http/HttpResponse.h"

// --- CgiProcess のメンバが public のため、前方宣言は不要 ---
// ただし、メイン関数内でポインタを使うため、全てのテスト関数を宣言します
void testPostSuccess();
void testGetSuccess();
void testStatus204NoContent();
void testStatus301Redirect();
void testImplicit302Redirect();
void testStatus400BadRequest();
void testStatus403Forbidden();
void testScriptNotFound();
void testStatus408Timeout();
void testScriptError();
void testBadCgiHeader();
void testWorkingDirAccess();
void testInfiniteLoopKill();
// ------------------------------------------

/**
 * @brief アサーションヘルパー。失敗時に例外を投げる。
 */
void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

/**
 * @brief HttpResponse の詳細をデバッグ出力する。
 */
void printResponse(const HttpResponse& response) {
    std::cout << "  Status: " << response.status_code_ << "\n";
    std::cout << "  Content-Type: " << response.header_.content_type_ << "\n";
    std::cout << "  Body:\n" << "    " << response.body_ << "\n";
    if (!response.location_.empty()) {
        std::cout << "  Location: " << response.location_ << "\n";
    }
}

/**
 * @brief リクエストのベースを作成する。
 */
HttpRequest createBaseRequest(Method method, const std::string& path,
                              const std::string& query) {
    HttpRequest request;
    request.method_ = method;
    request.request_target_.path_ = path;
    request.request_target_.query_string_ = query;
    request.body_ = "";
    request.header_.content_length_ = 0;
    return request;
}

HttpRequest createPostRequest() {
    HttpRequest request =
        createBaseRequest(MethodPOST, "/cgi-bin/test.py", "q=search");
    request.body_ = "body_from_request!!";
    request.header_.content_length_ = request.body_.length();
    request.header_.content_type_ = "text/plain";
    return request;
}

HttpRequest createGetRequest() {
    return createBaseRequest(MethodGET, "/cgi-bin/test.py",
                             "name=Test&test=123");
}

HttpRequest createNotFoundRequest() {
    return createBaseRequest(MethodGET, "/cgi-bin/non_existent_script.py", "");
}

HttpRequest createScriptErrorRequest() {
    return createBaseRequest(MethodGET, "/cgi-bin/error.py", "trigger=error");
}

HttpRequest createForbiddenRequest() {
    return createBaseRequest(MethodGET, "/cgi-bin/no_execute.py", "");
}

HttpRequest createTimeoutRequest() {
    return createBaseRequest(MethodGET, "/cgi-bin/timeout.py", "");
}

HttpRequest createHeaderTestRequest(const std::string& query) {
    return createBaseRequest(MethodGET, "/cgi-bin/test_headers.py", query);
}

// --- Test Cases ---

// 200 OK (POST)
void testPostSuccess() {
    CgiProcess process;
    HttpRequest request = createPostRequest();
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::OK, "Status code should be 200");
    check(response.header_.content_type_ == "text/html; charset=UTF-8",
          "Content-Type mismatch");
    check(response.body_.find("body_from_request!!") != std::string::npos,
          "Body missing stdin content");
    check(response.body_.find("Method: POST") != std::string::npos,
          "Missing POST method in env");
    check(response.body_.find("CONTENT_TYPE: text/plain") != std::string::npos,
          "Missing CONTENT_TYPE environment variable in body");
}

// 200 OK (GET)
void testGetSuccess() {
    CgiProcess process;
    HttpRequest request = createGetRequest();
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::OK, "Status code should be 200");
    check(response.header_.content_type_ == "text/html; charset=UTF-8",
          "Content-Type mismatch");
    check(response.body_.find("Method: GET") != std::string::npos,
          "Missing GET method in env");
    check(response.body_.find("QUERY_STRING:</td><td>name=Test&amp;test=123") !=
              std::string::npos,
          "Missing query string in env");
}

// 204 No Content
void testStatus204NoContent() {
    CgiProcess process;
    HttpRequest request = createHeaderTestRequest("status=204");
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::NoContent,
          "Status code should be 204");
    check(response.body_.empty() == true, "Body should be empty for 204");
}

// 301 Moved Permanently
void testStatus301Redirect() {
    CgiProcess process;
    HttpRequest request = createHeaderTestRequest("status=301");
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::MovedPermanently,
          "Status code should be 301");
    check(!response.location_.empty(), "Location header should be set");
    check(response.location_ == "http://www.google.com/",
          "Location header value mismatch");
}

// 302 Found (Implicit)
void testImplicit302Redirect() {
    CgiProcess process;
    HttpRequest request = createHeaderTestRequest("location_only=true");
    HttpResponse response = process.run(request);

    printResponse(response);

    // CGIスクリプトの出力に合わせて期待値を修正
    const std::string expected_location = "/test/new_resource";

    check(response.status_code_ == HttpStatus::Found,
          "Status code should be implicitly 302 Found");
    check(!response.location_.empty(), "Location header should be set");
    check(response.location_ == expected_location, "Location mismatch");
}

// 400 Bad Request
void testStatus400BadRequest() {
    CgiProcess process;
    HttpRequest request = createHeaderTestRequest("status=400");
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::BadRequest,
          "Status code should be 400");
    check(response.body_.find("Bad query parameter.") != std::string::npos,
          "Body should contain error message from script");
}

// 403 Forbidden
void testStatus403Forbidden() {
    CgiProcess process;
    HttpRequest request = createForbiddenRequest();
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::Forbidden,
          "Status code should be 403 Forbidden");
}

// 404 Not Found
void testScriptNotFound() {
    CgiProcess process;
    HttpRequest request = createNotFoundRequest();
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::NotFound,
          "Status code should be 404 Not Found");
}

// 408 Request Timeout (無限ループ/タイムアウトの検証)
void testStatus408Timeout() {
    CgiProcess process;
    HttpRequest request = createTimeoutRequest();
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::RequestTimeout,
          "Status code should be 408 RequestTimeout");
    check(response.body_.find("timed out") != std::string::npos,
          "Body should contain timeout message");
}

// 500 Internal Server Error (Script Crash)
void testScriptError() {
    CgiProcess process;
    HttpRequest request = createScriptErrorRequest();
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::InternalServerError,
          "Status code should be 500");
    check(response.body_.find("CGI script error") != std::string::npos,
          "Body should contain script error message");
}

// 500 Internal Server Error (Bad CGI Headers)
void testBadCgiHeader() {
    CgiProcess process;
    HttpRequest request = createHeaderTestRequest("status=bad_header");
    HttpResponse response = process.run(request);

    printResponse(response);

    check(response.status_code_ == HttpStatus::InternalServerError,
          "Status code should be 500");
    check(response.body_.find("malformed response") != std::string::npos,
          "Body should indicate malformed response");
}

// 1. 作業ディレクトリの検証 (CWD check)
void testWorkingDirAccess() {
    CgiProcess process;
    // cgi-bin/cwd_test.py がカレントディレクトリの情報を出力することを期待
    HttpRequest request =
        createBaseRequest(MethodGET, "/cgi-bin/cwd_test.py", "");
    HttpResponse response = process.run(request);

    printResponse(response);

    // CGI仕様: スクリプトはそれ自体があるディレクトリで実行されるべき
    check(response.body_.find("CWD: /src/cgi-bin") != std::string::npos,
          "CGI script did not run in its directory (CWD check failed)");
    check(response.status_code_ == HttpStatus::OK, "Status code should be 200");
}

// 2. 無限ループ/タイムアウト (サーバーがクラッシュしないことを確認) -
// testStatus408Timeout で代替済み

// 3. スクリプトエラー (ゼロ以外の終了コード) - testScriptError で代替済み

typedef void (*TestFunctionPtr)();

int main() {
    std::vector<std::pair<std::string, TestFunctionPtr> > tests;

    // --- 機能テスト ---
    tests.push_back(
        std::make_pair("testPostSuccess (200 OK)", testPostSuccess));
    tests.push_back(std::make_pair("testGetSuccess (200 OK)", testGetSuccess));
    tests.push_back(std::make_pair("testStatus204NoContent (204 No Content)",
                                   testStatus204NoContent));
    tests.push_back(std::make_pair("testStatus301Redirect (301 Redirect)",
                                   testStatus301Redirect));
    tests.push_back(std::make_pair("testImplicit302Redirect (302 Implicit)",
                                   testImplicit302Redirect));

    // --- 堅牢性/エラー耐性テスト ---
    tests.push_back(std::make_pair("testWorkingDirAccess (CWD check)",
                                   testWorkingDirAccess));
    tests.push_back(std::make_pair("testStatus408Timeout (408 Timeout)",
                                   testStatus408Timeout));
    tests.push_back(
        std::make_pair("testScriptError (500 Exit Code)", testScriptError));
    tests.push_back(
        std::make_pair("testBadCgiHeader (500 Bad Header)", testBadCgiHeader));
    tests.push_back(std::make_pair("testStatus400BadRequest (400 Bad Request)",
                                   testStatus400BadRequest));
    tests.push_back(std::make_pair("testStatus403Forbidden (403 Forbidden)",
                                   testStatus403Forbidden));
    tests.push_back(std::make_pair("testScriptNotFound (404 Not Found)",
                                   testScriptNotFound));

    int passed = 0;
    int failed = 0;

    std::cout
        << "======== Running CgiProcess Tests (Comprehensive) ========\n\n";

    for (std::size_t i = 0; i < tests.size(); ++i) {
        const std::string& testName = tests[i].first;
        TestFunctionPtr testFunc = tests[i].second;

        std::cout << "--- Running: " << testName << " ---\n";
        try {
            testFunc();
            std::cout << "\n[SUCCESS] " << testName << " PASSED!\n";
            passed++;
        } catch (const std::exception& e) {
            std::cout << "\n[FAILURE] " << testName << " FAILED:\n  "
                      << e.what() << "\n";
            failed++;
        }
        std::cout << "------------------------------------------\n\n";
    }

    std::cout << "============= Test Summary =============\n";
    std::cout << "Total: " << tests.size() << ", Passed: " << passed
              << ", Failed: " << failed << "\n";
    std::cout << "========================================\n\n";

    return (failed > 0) ? 1 : 0;
}
