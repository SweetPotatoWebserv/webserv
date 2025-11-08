#include "HttpConfigParser.hpp"
#include <iostream>
#include <vector>

/*
** Day 1 のゴール（トークナイザーの確認）を実行するメイン関数
*/
// int main(int argc, char **argv) {
//     // 実行時に .conf ファイルが指定されているかチェック
//     if (argc != 2) {
//         std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
//         return 1;
//     }

//     std::string filename = argv[1];

//     try {
//         // タスク4で実装するコンストラクタを呼び出す
//         HttpConfigParser parser(filename);

//         // トークンリストを取得して表示
//         // ※注: HttpConfigParser.hpp に getTokens() を追加する必要があります
//         const std::vector<std::string>& tokens = parser.getTokens();

//         std::cout << "--- Tokenizer Result ---" << std::endl;
//         std::cout << "[";
//         for (size_t i = 0; i < tokens.size(); ++i) {
//             std::cout << "\"" << tokens[i] << "\"";
//             if (i < tokens.size() - 1) {
//                 std::cout << ", ";
//             }
//         }
//         std::cout << "]" << std::endl;
//         std::cout << "------------------------" << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }

#include "HttpConfigParser.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        HttpConfigParser parser(argv[1]);
        
        std::cout << "--- Tokens ---" << std::endl;
        parser.printTokens(); // デバッグ関数を呼ぶ
        std::cout << "--------------" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}