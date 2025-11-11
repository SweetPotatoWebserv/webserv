#include <gtest/gtest.h>
#include "../src/http/ClientHandler.h"

TEST(IsCompleteContentLength, ShortBodyFalse) {
    std::string message_head = std::string("POST / HTTP/1.1") + HTTP_LINE_END + std::string("host: localhost") + HTTP_LINE_END + "content-length: 5"; // NOLINT
    std::string::size_type content_pos = message_head.find(ClientHandler::CONTENT_LENGTH_WITH_COLON);
    std::string buffer = message_head + HTTP_HEADER_END + "1234";  // NOLINT
    EXPECT_FALSE(ClientHandler::is_complete_content_length(buffer, message_head, content_pos));
}

TEST(IsCompleteContentLength, FullBodyTrue) {
    std::string message_head = std::string("POST / HTTP/1.1") + HTTP_LINE_END + std::string("host: localhost") + HTTP_LINE_END + "content-length: 5";  // NOLINT
    std::string::size_type content_pos = message_head.find(ClientHandler::CONTENT_LENGTH_WITH_COLON);
    std::string buffer = message_head + HTTP_HEADER_END + "12345";  // NOLINT
    EXPECT_TRUE(ClientHandler::is_complete_content_length(buffer, message_head, content_pos));
}

TEST(IsCompleteContentLength, SpacesAndThreeBytesTrue) {
    std::string headers_ws = std::string("host: localhost") + HTTP_LINE_END +
                             "content-length:    3";  // NOLINT
    std::string::size_type pos_ws = headers_ws.find(ClientHandler::CONTENT_LENGTH_WITH_COLON);
    std::string buffer = std::string("POST / HTTP/1.1") + HTTP_LINE_END +
                         headers_ws + HTTP_HEADER_END + "abc";
    EXPECT_TRUE(ClientHandler::is_complete_content_length(buffer, headers_ws,
                                                          pos_ws));
}
