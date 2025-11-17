#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <fstream>    // file read
#include <sstream>    // file read
#include <stdexcept>  // exception
#include <string>
#include <vector>

#include "HttpConfig.h"  // データ保持クラス

// 設定ファイル(.conf)のパース(解析)静的解析クラス

class HttpConfigParser {
   public:
    //@brief パースを実行する唯一の public インターフェース
    //@param filename 設定ファイルパス
    //@return 完成した HttpConfig オブジェクト
    static HttpConfig parse(const std::string& filename);

   private:
    HttpConfigParser();
    ~HttpConfigParser();  // instance制限のためprivate

    //@brief ファイルを読み込み、トークン化する
    //@param filename ファイルパス
    //@return トークンのリスト
    static std::vector<std::string> tokenize(const std::string& filename);

    //@brief 終端に来たかどうか
    // @param tokens トークンリスト
    // @param index 現在の位置
    // @return 終端なら true
    static bool isEof(const std::vector<std::string>& tokens, size_t index);

    //@brief 次のトークンを取得し、インデックスを1つ進める
    //  @param tokens トークンリスト
    //  @param index 現在の位置 (参照渡しで、この値が内部で +1 される)
    //  @return 次のトークン
    static std::string getNextToken(const std::vector<std::string>& tokens,
                                    size_t& index);

    //@brief serverブロックをパースする
    //@param config HttpConfigオブジェクト（パース結果を格納）
    //@param tokens トークンリスト
    //@param index 現在の位置（参照渡しで進める）
    static void parserServer(HttpConfig& config,
                             const std::vector<std::string>& tokens,
                             size_t& index);
    //@brief locationブロックをパースする
    //@param server_config ServerConfigオブジェクト（パース結果を格納）
    //@param tokens トークンリスト
    //@param index 現在の位置（参照渡しで進める）
    static void parserLocation(ServerConfig& server_config,
                               const std::vector<std::string>& tokens,
                               size_t& index);

    ///@brief listen ディレクティブをパースする
    ///@return 完成した ListenDirective オブジェクト
    static ListenDirective parseListen(const std::vector<std::string>& tokens,
                                       size_t& index);

    //@brief root ディレクティブをパースする
    //@return root のパス　"/var/www/html"など
    static std::string parseRoot(const std::vector<std::string>& tokens,
                                 size_t& index);

    //@brief index ディレクティブをパースする (複数形対応)
    //@return index ファイル名のリスト""index.html"など
    static std::vector<std::string> parseIndex(
        const std::vector<std::string>& tokens, size_t& index);

    //@brief autoindex ディレクティブをパースする
    //@return autoindex のオンオフ(true/false)
    static bool parseAutoindex(const std::vector<std::string>& tokens,
                               size_t& index);

    ///@brief client_max_body_size ディレクティブをパースする
    ///@return ボディサイズの上限値
    static off_t parseClientMaxBodySize(const std::vector<std::string>& tokens,
                                        size_t& index);

    ///@brief listenのポート番号を検証し、uint16_tに変換するヘルパー関数
    static uint16_t validateAndConvertPort(uint64_t temp_port,
                                           const std::string& error_value);

    // parseErrorPage は複数の引数を返す必要があるため、
    // 専用のヘルパー構造体を定義
    struct ParsedErrorPage {
        std::vector<int> status_codes;
        ErrorPageDirective directive;
    };

    ///@brief error_page ディレクティブをパースする
    ///@return ParsedErrorPage (ステータスコードのリストと、設定内容)
    static ParsedErrorPage parseErrorPage(
        const std::vector<std::string>& tokens, size_t& index);

    ///@brief return ディレクティブをパースする
    ///@return 完成した ReturnDirective オブジェクト
    static ReturnDirective parseReturn(const std::vector<std::string>& tokens,
                                       size_t& index);

    // 特殊文字リスト（トークン分割用）構文解析用
    static const char* const SPECIAL_CHARS;
    static const char HASH_CHAR;
    static const char* const KEYWORD_HTTP;
    static const char* const KEYWORD_SERVER;
    static const char* const KEYWORD_LOCATION;
    static const char* const BRACE_OPEN;
    static const char* const BRACE_CLOSE;
    static const char* const SEMICOLON;

    // Directive keywords
    static const char* const DIRECTIVE_LISTEN;
    static const char* const DIRECTIVE_ROOT;
    static const char* const DIRECTIVE_INDEX;
    static const char* const DIRECTIVE_AUTOINDEX;
    static const char* const DIRECTIVE_CLIENT_MAX_BODY_SIZE;
    static const char* const DIRECTIVE_ERROR_PAGE;
    static const char* const DIRECTIVE_RETURN;
    // parserlisten defult
    static const char* const KEYWORD_DEFAULT_SERVER;
    // on,off
    static const char* const VALUE_ON;
    static const char* const VALUE_OFF;
    // ClientMaxBodySize
    static const char SUFFIX_KILOBYTE;
    static const char SUFFIX_MEGABYTE;
    static const char SUFFIX_GIGABYTE;

    static const char* HTTP_PREFIX;
    static const char* HTTPS_PREFIX;

    // マジックナンバー定数に
    static const off_t BYTES_PER_KB = 1024;
    static const off_t BYTES_PER_MB = static_cast<off_t>(1024) * 1024;
    static const off_t BYTES_PER_GB = static_cast<off_t>(1024) * 1024 * 1024;

    static const int MIN_OVERRIDE_STATUS_CODE = 200;
    static const int MAX_OVERRIDE_STATUS_CODE = 501;
    static const int MIN_RETURN_STATUS_CODE = 0;
    static const int MAX_RETURN_STATUS_CODE = 501;

    template <typename T>
    static bool parseCommonDirective(T& config, const std::string& token,
                                     const std::vector<std::string>& tokens,
                                     size_t& index) {
        if (token == DIRECTIVE_ROOT) {
            std::string r = parseRoot(tokens, index);
            config.setRoot(r);
        } else if (token == DIRECTIVE_INDEX) {
            std::vector<std::string> files = parseIndex(tokens, index);
            for (size_t i = 0; i < files.size(); ++i) {
                config.addIndexFile(files[i]);
            }
        } else if (token == DIRECTIVE_AUTOINDEX) {
            bool ai = parseAutoindex(tokens, index);
            config.setAutoindex(ai);
        } else if (token == DIRECTIVE_CLIENT_MAX_BODY_SIZE) {
            off_t size = parseClientMaxBodySize(tokens, index);
            config.setClientMaxBodySize(size);
        } else if (token == DIRECTIVE_ERROR_PAGE) {
            ParsedErrorPage pep = parseErrorPage(tokens, index);
            // 取得した全てのステータスコードについて、map に登録
            for (size_t i = 0; i < pep.status_codes.size(); ++i) {
                config.addErrorPage(pep.status_codes[i], pep.directive);
            }
        } else {
            // 共通ディレクティブではなかった
            return false;
        }
        // 共通ディレクティブのどれかとして処理が成功した
        return true;
    }
};