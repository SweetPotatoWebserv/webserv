#include <gtest/gtest.h>
#include "../src/http/ClientHandler.h"

TEST(IsRequestReady, HeaderOnlyReady) {
    std::string req = std::string("GET / HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost" + HTTP_LINE_END + HTTP_LINE_END;
    EXPECT_TRUE(ClientHandler::is_request_ready(req));
}

TEST(IsRequestReady, ContentLengthShortBodyNotReady) {
    std::string req = std::string("POST / HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost" + HTTP_LINE_END +
                      "Content-Length: 5" + HTTP_LINE_END + HTTP_LINE_END +
                      "1234";  // NOLINT
    EXPECT_FALSE(ClientHandler::is_request_ready(req));
}

TEST(IsRequestReady, ContentLengthFullBodyReady) {
    std::string req = std::string("POST / HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost" + HTTP_LINE_END +
                      "Content-Length: 5" + HTTP_LINE_END + HTTP_LINE_END +
                      "12345";  // NOLINT
    EXPECT_TRUE(ClientHandler::is_request_ready(req));
}

TEST(IsRequestReady, TransferEncodingChunkedWithoutTerminalNotReady) {
    std::string req = std::string("POST / HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost" + HTTP_LINE_END +
                      "Transfer-Encoding: chunked" + HTTP_LINE_END +
                      HTTP_LINE_END;  // NOLINT
    EXPECT_FALSE(ClientHandler::is_request_ready(req));
}

TEST(IsRequestReady, TransferEncodingChunkedWithBody){
    std::string req = std::string("POST / HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost" + HTTP_LINE_END +
                      "Transfer-Encoding: chunked" + HTTP_LINE_END +
                      HTTP_LINE_END +
                      "5\r\nHello\r\n0\r\n\r\n";  // NOLINT
    EXPECT_TRUE(ClientHandler::is_request_ready(req));
}

TEST(IsRequestReady, TransferEncodingNonChunkedIsReady) {
    std::string req = std::string("POST / HTTP/1.1") + HTTP_LINE_END +
                      "Host: localhost" + HTTP_LINE_END +
                      "Transfer-Encoding: gzip" + HTTP_LINE_END +
                      HTTP_LINE_END;  // NOLINT
    EXPECT_TRUE(ClientHandler::is_request_ready(req));
}
