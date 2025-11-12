#include <gtest/gtest.h>
#include "../src/core/String.h"

TEST(SearchHeaderField, FindsTransferEncodingChunked) {
    std::string head = std::string("host: localhost") + HTTP_LINE_END +
                       "transfer-encoding: chunked" + HTTP_LINE_END +
                       "content-length: 10";
    std::vector<std::string> found;
    bool ok = search_header_field(head, TRANSFER_ENCODING, found);
    ASSERT_TRUE(ok);
    ASSERT_EQ(found.size(), static_cast<size_t>(HEADER_FIELD_NUM));
    EXPECT_EQ(found[0], std::string(TRANSFER_ENCODING));
    EXPECT_EQ(found[1], std::string("chunked"));
}

TEST(SearchHeaderField, TrimsContentLengthValueSpaces) {
    std::string head = std::string("host: localhost") + HTTP_LINE_END +
                       "content-length:    5   " + HTTP_LINE_END;
    std::vector<std::string> found;
    bool ok = search_header_field(head, CONTENT_LENGTH, found);
    ASSERT_TRUE(ok);
    ASSERT_EQ(found.size(), static_cast<size_t>(HEADER_FIELD_NUM));
    EXPECT_EQ(found[0], std::string(CONTENT_LENGTH));
    EXPECT_EQ(found[1], std::string("5"));
}

TEST(SearchHeaderField, MissingHeaderReturnsFalse) {
    std::string head = std::string("host: localhost") + HTTP_LINE_END +
                       "content-type: text/plain" + HTTP_LINE_END;
    std::vector<std::string> found;
    bool ok = search_header_field(head, TRANSFER_ENCODING, found);
    EXPECT_FALSE(ok);
}

TEST(SearchHeaderField, EmptyValueKeepsEmpty) {
    std::string head = std::string("host: localhost") + HTTP_LINE_END +
                       "content-length:" + HTTP_LINE_END;
    std::vector<std::string> found;
    bool ok = search_header_field(head, CONTENT_LENGTH, found);
    ASSERT_FALSE(ok);
}

