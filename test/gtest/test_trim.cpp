#include <gtest/gtest.h>
#include "../src/core/String.h"

TEST(StringTrim, LeadingAndTrailingSpaces) {
    std::string s = "  hello world  ";
    EXPECT_EQ(trim(s), "hello world");
}

TEST(StringTrim, NoTrimNeeded) {
    std::string s = "hello";
    EXPECT_EQ(trim(s), "hello");
}

TEST(StringTrim, AllSpacesBecomesEmpty) {
    std::string s = "   \t\n\r\v\f   ";
    EXPECT_EQ(trim(s), "");
}

TEST(StringTrim, TabsAndNewlines) {
    std::string s = "\n\t abc def \r\n";
    EXPECT_EQ(trim(s), "abc def");
}

TEST(StringTrim, InteriorSpacesRemain) {
    std::string s = "  a  b  c  ";
    EXPECT_EQ(trim(s), "a  b  c");
}

TEST(StringTrim, EmptyString) {
    std::string s = "";
    EXPECT_EQ(trim(s), "");
}

TEST(StringTrim, SingleWhitespaceChar) {
    std::string s = "\t";
    EXPECT_EQ(trim(s), "");
}

