#include <gtest/gtest.h>
#include "../src/http/ClientHandler.h"

// chunked: 終端 0\r\n\r\n が無ければ未完(false)
TEST(IsCompleteTransfer, ChunkedWithoutTerminalIsFalse) {
    std::string head = std::string("host: a") + HTTP_LINE_END +
                       "transfer-encoding: chunked";
    std::string buffer = head + HTTP_HEADER_END + "4\r\nWiki\r\n"; // 中途
    std::string::size_type pos = head.find(ClientHandler::TRANSFER_ENCODING_WITH_COLON);
    EXPECT_FALSE(ClientHandler::is_complete_transfer(buffer, head, pos));
}

// chunked: 終端 0\r\n\r\n があれば完了(true)
TEST(IsCompleteTransfer, ChunkedWithTerminalIsTrue) {
    std::string head = std::string("host: a") + HTTP_LINE_END +
                       "transfer-encoding: chunked";
    std::string buffer = head + HTTP_HEADER_END + "0\r\n\r\n";
    std::string::size_type pos = head.find(ClientHandler::TRANSFER_ENCODING_WITH_COLON);
    EXPECT_TRUE(ClientHandler::is_complete_transfer(buffer, head, pos));
}

// 非chunkedのTEは本文不要で即完了(true)
TEST(IsCompleteTransfer, NonChunkedIsTrue) {
    std::string head = std::string("host: a") + HTTP_LINE_END +
                       "transfer-encoding: gzip";
    std::string buffer = head + HTTP_HEADER_END;
    std::string::size_type pos = head.find(ClientHandler::TRANSFER_ENCODING_WITH_COLON);
    EXPECT_TRUE(ClientHandler::is_complete_transfer(buffer, head, pos));
}
