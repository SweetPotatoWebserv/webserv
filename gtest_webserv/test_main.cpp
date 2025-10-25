#include "gtest/gtest.h"  //<-- GTestのヘッダーをインクルード

// --- (ひな型) webservのヘッダーをインクルード ---
//(ex #include "ConfigParser.hpp"
//(ex #include "RequestParser.hpp"
// ... 他にもテストしたいクラスがあればここに追加 ...

// --- サンプルテスト (動作確認用) ---
TEST(SampleTest, Addition) { EXPECT_EQ(4, 2 + 2); }

// --- (ひな型) ConfigParser のテスト例 ---
/*
TEST(ConfigParserTest, ShouldHandleSimpleConf) {
    ConfigParser parser;

    // (注意: このテストを実行する前に、
    // tests/configs/simple.conf
のようなテスト用設定ファイルを用意する必要があります) bool success =
parser.loadFile("tests/configs/simple.conf");

    // 1. まず、ファイルが正常に読み込めたことを確認
    ASSERT_TRUE(success);

    // 2. 期待する値が取れているか確認
    EXPECT_EQ(8080, parser.getPort());
    EXPECT_EQ("/var/www", parser.getRoot());
}
*/

// --- (ひな型) RequestParser のテスト例 ---
/*
TEST(RequestParserTest, ShouldParseGetMethod) {
    RequestParser parser;
    HTTPRequest request; // (仮のクラス名)

    // テスト用の生リクエスト文字列
    std::string rawRequest = "GET /index.html HTTP/1.1\r\n"
                             "Host: example.com\r\n"
                             "\r\n";

    parser.parse(request, rawRequest);

    EXPECT_EQ("GET", request.getMethod());
    EXPECT_EQ("/index.html", request.getPath());
    EXPECT_EQ("example.com", request.getHeader("Host"));
}
*/

// --- GTestを実行するためのmain関数 (※これは触らない) ---
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}