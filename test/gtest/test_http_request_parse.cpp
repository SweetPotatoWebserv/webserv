#include <gtest/gtest.h>
#include "../src/http/HttpParser.h"
#include "../src/core/Common.h"
#include "../src/http/ClientHandler.h"
#include "../src/http/HttpException.h"

// GET: method, host(no port -> DEFAULT_PORT), request_target(path+query), no body
TEST(HttpRequestParse, ParseRequestLineAndHostNoPort) {
    std::string req = std::string("GET /index.html?x=1&y=2 HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodGET);
    EXPECT_EQ(r.request_target_.path_, "/index.html");
    EXPECT_EQ(r.request_target_.query_string_, "x=1&y=2");
    EXPECT_EQ(r.host_.getAddress(), std::string("example.com"));
    EXPECT_EQ(r.host_.getPort(), DEFAULT_PORT);
    EXPECT_TRUE(r.body_.empty());
}

// POST + Content-Length + Content-Type + explicit host:port
TEST(HttpRequestParse, ParseContentLengthAndTypeAndBody) {
    std::string body = "Hello, world!"; // 13 bytes // NOLINT
    std::string req = std::string("POST /submit HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost:8080" + HTTP_LINE_END +
                      "Content-Type: application/json" + HTTP_LINE_END +
                      "Content-Length: 13" + HTTP_LINE_END + // NOLINT
                      HTTP_LINE_END +
                      body;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodPOST);
    EXPECT_EQ(r.request_target_.path_, "/submit");
    EXPECT_EQ(r.host_.getAddress(), std::string("localhost"));
    EXPECT_EQ(r.host_.getPort(), static_cast<uint16_t>(8080)); // NOLINT
    EXPECT_EQ(r.header_.content_type_, std::string("application/json"));
    EXPECT_EQ(r.header_.content_length_, static_cast<std::size_t>(13)); // NOLINT
    EXPECT_EQ(r.body_, body);
}

// Transfer-Encoding: chunked (only chunked is supported). Expect decoded body and flag.
TEST(HttpRequestParse, ParseTransferEncodingChunked) {
    // "Wikipedia" in two chunks: 4+5
    std::string req = std::string("POST /chunk HTTP/1.1") + HTTP_LINE_END +
                      "Host: example.com" + HTTP_LINE_END +
                      "Transfer-Encoding: chunked" + HTTP_LINE_END +
                      HTTP_LINE_END +
                      "4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n"; // NOLINT

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    // method + target
    EXPECT_EQ(r.method_, MethodPOST);
    EXPECT_EQ(r.request_target_.path_, "/chunk");
    // transfer-encoding should include "chunked"
    ASSERT_FALSE(r.header_.transfer_encoding_.empty());
    bool has_chunked = (r.header_.transfer_encoding_ == "chunked");
    EXPECT_TRUE(has_chunked);
    // decoded body
    EXPECT_EQ(r.body_, std::string("Wikipedia"));
}

TEST(HttpRequestParse, InvalidHostAddress) {
    std::string req = std::string("GET /index.html HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));

    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodGET);
    EXPECT_EQ(r.request_target_.path_, "/index.html");
    EXPECT_EQ(r.host_.getAddress(), std::string("example.com"));
    EXPECT_EQ(r.host_.getPort(), 0);
    EXPECT_TRUE(r.body_.empty());
}

TEST(HttpRequestParse, InvalidHostPort) {
    std::string req = std::string("GET /index.html HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: :8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodGET);
    EXPECT_EQ(r.request_target_.path_, "/index.html");
    EXPECT_EQ(r.host_.getAddress(), std::string(""));
    EXPECT_EQ(r.host_.getPort(), 8080);
    EXPECT_TRUE(r.body_.empty());
}

TEST(HttpRequestParse, InvalidRequestLineNotVersion) {
    std::string req = std::string("GET /index.html") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    ASSERT_THROW(HttpParser::http_request_parse(req), HttpException);
}

TEST(HttpRequestParse, InvalidRequestLineNotMethod) {
    std::string req = std::string("/index.html HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    ASSERT_THROW(HttpParser::http_request_parse(req), HttpException);
}

TEST(HttpRequestParse, InvalidRequestLineNotPath) {
    std::string req = std::string("GET HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    ASSERT_THROW(HttpParser::http_request_parse(req), HttpException);
}

TEST(HttpRequestParse, InvalidRequestLineNotPathWithSpace) {
    std::string req = std::string("GET  HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    ASSERT_THROW(HttpParser::http_request_parse(req), HttpException);
}

TEST(HttpRequestParse, InvalidRequestLineLowerMethod) {
    std::string req = std::string("get / HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    ASSERT_THROW(HttpParser::http_request_parse(req), HttpException);
}


TEST(HttpRequestParse, QueryStringLastQuestion) {
    std::string req = std::string("GET /? HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodGET);
    EXPECT_EQ(r.request_target_.path_, "/");
    EXPECT_EQ(r.request_target_.query_string_, "");
    EXPECT_EQ(r.host_.getAddress(), std::string("example.com"));
    EXPECT_EQ(r.host_.getPort(), 8080);
    EXPECT_TRUE(r.body_.empty());
}

TEST(HttpRequestParse, OnlyQuestionPath) {
    std::string req = std::string("GET ? HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodGET);
    EXPECT_EQ(r.request_target_.path_, "");
    EXPECT_EQ(r.request_target_.query_string_, "");
    EXPECT_EQ(r.host_.getAddress(), std::string("example.com"));
    EXPECT_EQ(r.host_.getPort(), 8080);
    EXPECT_TRUE(r.body_.empty());
}

TEST(HttpRequestParse, DuplicateQuestion) {
    std::string req = std::string("GET /??name=tarou HTTP/1.1") +
                      HTTP_LINE_END +
                      "Host: example.com:8080" + HTTP_LINE_END +
                      HTTP_LINE_END;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    HttpRequest r = HttpParser::http_request_parse(req);

    EXPECT_EQ(r.method_, MethodGET);
    EXPECT_EQ(r.request_target_.path_, "/");
    EXPECT_EQ(r.request_target_.query_string_, "?name=tarou");
    EXPECT_EQ(r.host_.getAddress(), std::string("example.com"));
    EXPECT_EQ(r.host_.getPort(), 8080);
    EXPECT_TRUE(r.body_.empty());
}

TEST(HttpRequestParse, ParseNoContentLengthWithBody) {
    std::string body = "Hello, world!"; // 13 bytes // NOLINT
    std::string req = std::string("POST /submit HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost:8080" + HTTP_LINE_END +
                      "Content-Type: application/json" + HTTP_LINE_END +
                      HTTP_LINE_END +
                      body;

    ASSERT_TRUE(ClientHandler::is_request_ready(req));
    ASSERT_THROW(HttpParser::http_request_parse(req), HttpException);
}
