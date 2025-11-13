#include <gtest/gtest.h>
#include "../src/core/String.h"

TEST(StringSplit, BasicSpaceSeparated) {
    std::string s = "a b c";
    std::vector<std::string> parts = split(s);
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
}

TEST(StringSplit, ConsecutiveSeparatorsProduceEmpty) {
    std::string s = "a  b";
    std::vector<std::string> parts = split(s);
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "");
    EXPECT_EQ(parts[2], "b");
}

TEST(StringSplit, MultiCharSeparator) {
    std::string s = "a, b, c";
    std::vector<std::string> parts = split(s, ", ");
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
}

TEST(StringSplit, NoSeparatorReturnsWhole) {
    std::string s = "abc";
    std::vector<std::string> parts = split(s, ",");
    ASSERT_EQ(parts.size(), 1u);
    EXPECT_EQ(parts[0], "abc");
}

TEST(StringSplit, TrailingSeparatorProducesEmptyTail) {
    std::string s = "a,b,";
    std::vector<std::string> parts = split(s, ",");
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "");
}

TEST(StringSplit, LeadingSeparatorProducesEmptyHead) {
    std::string s = ",a";
    std::vector<std::string> parts = split(s, ",");
    ASSERT_EQ(parts.size(), 2u);
    EXPECT_EQ(parts[0], "");
    EXPECT_EQ(parts[1], "a");
}

TEST(StringSplit, EmptyInputReturnsSingleEmpty) {
    std::string s = "";
    std::vector<std::string> parts = split(s, ",");
    ASSERT_EQ(parts.size(), 1u);
    EXPECT_EQ(parts[0], "");
}

TEST(StringSplit, QueryStringAnd) {
    std::string s = "name=tarou&age=17";
    std::vector<std::string> parts = split(s, "&");
    ASSERT_EQ(parts.size(), 2u);
    EXPECT_EQ(parts[0], "name=tarou");
    EXPECT_EQ(parts[1], "age=17");
}

TEST(StringSplit, QueryStringEqual) {
    std::string s = "name=tarou";
    std::vector<std::string> parts = split(s, "=");
    ASSERT_EQ(parts.size(), 2u);
    EXPECT_EQ(parts[0], "name");
    EXPECT_EQ(parts[1], "tarou");
}

TEST(StringSplit, QueryStringQuestion) {
    std::string s = "?";
    std::vector<std::string> parts = split(s, "?");
    ASSERT_EQ(parts.size(), 2u);
    EXPECT_EQ(parts[0], "");
    EXPECT_EQ(parts[1], "");
}
