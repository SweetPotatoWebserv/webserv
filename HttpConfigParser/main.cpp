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

// main.cpp
#include "HttpConfigParser.hpp" // レシピ本をインクルード
#include "HttpConfig.h"       // 完成品のお盆もインクルード
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        // --- ↓↓↓ ここが決定的に変わる ↓↓↓ ---
        //
        // [旧] HttpConfigParser parser(argv[1]);
        // [旧] HttpConfig config = parser.getConfig();
        //
        // [新] HttpConfigParser クラスから、直接 ::parse() を呼び出す
        
        HttpConfig config = HttpConfigParser::parse(argv[1]); 
        
        // --- ↑↑↑ ここまで ---

        // Day 2 の完了確認
        std::cout << "Config file parsed successfully!" << std::endl;

        // (デバッグ用：config に何が入ったか確認するコードを将来追加できる)

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}