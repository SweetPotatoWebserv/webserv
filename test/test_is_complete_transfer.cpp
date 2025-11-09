#include <gtest/gtest.h>
#include "../src/http/ClientHandler.h"

TEST(IsCompleteTransfer, SimpleChunkedTrue) {
    std::string head = std::string("host: a") + HTTP_LINE_END +
                       "transfer-encoding: chunked";
    std::string::size_type pos = head.find(TRANSFER_ENCODING);
    EXPECT_TRUE(ClientHandler::is_complete_transfer(head, pos));
}

TEST(IsCompleteTransfer, IncludesChunkedTrue) {
    std::string head = std::string("host: a") + HTTP_LINE_END +
                       "transfer-encoding: gzip, chunked";
    std::string::size_type pos = head.find(TRANSFER_ENCODING);
    EXPECT_TRUE(ClientHandler::is_complete_transfer(head, pos));
}

TEST(IsCompleteTransfer, NoChunkedFalse) {
    std::string head = std::string("host: a") + HTTP_LINE_END +
                       "transfer-encoding: gzip";
    std::string::size_type pos = head.find(TRANSFER_ENCODING);
    EXPECT_FALSE(ClientHandler::is_complete_transfer(head, pos));
}

