#include "HttpConfigParser.h"

#include <cctype>   // std::isspace
#include <limits>   // std::numeric_limits を使うために必要
#include <sstream>  //parseListen用

//構文解析用クラスkeyword
const char* const HttpConfigParser::SPECIAL_CHARS = "{};";
const char HttpConfigParser::HASH_CHAR = '#';
const char* const HttpConfigParser::KEYWORD_HTTP = "http";
const char* const HttpConfigParser::KEYWORD_SERVER = "server";
const char* const HttpConfigParser::KEYWORD_LOCATION = "location";
const char* const HttpConfigParser::BRACE_OPEN = "{";
const char* const HttpConfigParser::BRACE_CLOSE = "}";
const char* const HttpConfigParser::SEMICOLON = ";";
// Directive keywords
const char* const HttpConfigParser::DIRECTIVE_LISTEN = "listen";
const char* const HttpConfigParser::DIRECTIVE_ROOT = "root";
const char* const HttpConfigParser::DIRECTIVE_INDEX = "index";
const char* const HttpConfigParser::DIRECTIVE_AUTOINDEX = "autoindex";
const char* const HttpConfigParser::DIRECTIVE_CLIENT_MAX_BODY_SIZE =
    "client_max_body_size";
const char* const HttpConfigParser::DIRECTIVE_ERROR_PAGE = "error_page";
// parseListen
const char* const HttpConfigParser::KEYWORD_DEFAULT_SERVER = "default_server";
// on,off->autoindex
const char* const HttpConfigParser::VALUE_ON = "on";
const char* const HttpConfigParser::VALUE_OFF = "off";
// ClientMaxBodySize
const char HttpConfigParser::SUFFIX_KILOBYTE = 'k';
const char HttpConfigParser::SUFFIX_MEGABYTE = 'm';
const char HttpConfigParser::SUFFIX_GIGABYTE = 'g';

//終端に来たかどうか
bool HttpConfigParser::isEof(const std::vector<std::string>& tokens,
                             size_t index) {
    return index >= tokens.size();
}

//次のトークンを取得し、インデックスを1つ進める
std::string HttpConfigParser::getNextToken(
    const std::vector<std::string>& tokens, size_t& index) {
    if (isEof(tokens, index)) {
        throw std::runtime_error("Error: Unexpected end of file");
    }
    return tokens[index++];
}

//ファイル読み込みとトークン化
std::vector<std::string> HttpConfigParser::tokenize(
    const std::string& filename) {
    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open()) {
        throw std::runtime_error("Error: Could not open file " + filename);
    }

    std::vector<std::string> tokens;
    std::string line;

    // copilot
    const std::string special_chars_str(SPECIAL_CHARS);

    while (std::getline(ifs, line)) {
        for (size_t i = 0; i < line.size(); ++i) {
            // 空欄スキップ
            if (std::isspace(line[i])) {
                continue;
            }
            // コメント行スキップ
            if (line[i] == HASH_CHAR) {
                break;
            }  // HASH_CHAR==#
            // トークン抽出
            if (special_chars_str.find(line[i]) != std::string::npos) {
                tokens.push_back(
                    line.substr(i, 1));  // SPECIAL_CHARS=="{};"//1文字切り出す
                continue;
            }
            size_t start = i;
            while (i < line.size() && std::isspace(line[i]) == 0 &&
                   line[i] != HASH_CHAR &&
                   special_chars_str.find(line[i]) == std::string::npos) {
                ++i;
            }
            tokens.push_back(line.substr(start, i - start));
            --i;
        }
    }
    return tokens;
}

//-----------------------------------------------------------
//------------------parser本体-------------------------------
//-----------------------------------------------------------
///@brief パースを実行する唯一の public インターフェース
///@param filename 設定ファイルパス
///@return 完成した HttpConfig オブジェクト

HttpConfig HttpConfigParser::parse(const std::string& filename) {
    std::vector<std::string> tokens = HttpConfigParser::tokenize(filename);

    // parse処理に必要な道具をローカルに準備
    HttpConfig config;  //完成させるHttpConfigオブジェクト
    size_t index = 0;   //現在のトークン位置

    //最初のトークンが "http" であることを確認
    if (isEof(tokens, index) || getNextToken(tokens, index) != KEYWORD_HTTP) {
        throw std::runtime_error(
            "Error: Expected 'http' block at the beginning");
    }

    // 次のトークンが "{" であることを確認
    if (isEof(tokens, index) ||
        getNextToken(tokens, index) != BRACE_OPEN) {  //{
        throw std::runtime_error("Error: Expected '{' after http directive");
    }

    // http ブロックの中身をパース
    bool found_closing_brace = false;

    while (!isEof(tokens, index)) {
        std::string token = getNextToken(tokens, index);

        if (token == BRACE_CLOSE) {
            // http ブロック終了
            found_closing_brace = true;
            break;
        }

        if (token == KEYWORD_SERVER) {
            // "server" が来たら、parserServer に任せる
            // (※ parserServerの中で config.addServerConfig される)
            HttpConfigParser::parserServer(config, tokens, index);
        } else if (HttpConfigParser::parseCommonDirective(config, token, tokens,
                                                          index)) {
            continue;
        } else {
            throw std::runtime_error(
                "Error: Unknown directive in http block: " + token);
        }
    }

    if (!found_closing_brace) {
        throw std::runtime_error("Error: Expected '}' to close http block");
    }

    return config;
}

//-----------------------------------------------------------
//------------------サーバーブロックパーサー-------------------
//-----------------------------------------------------------

///@brief server ブロックをパースする
///@param config HttpConfig オブジェクト (参照渡し)
///@param tokens トークンリスト
///@param index 現在の位置 (参照渡しで、この値が内部で進められる)

void HttpConfigParser::parserServer(HttpConfig& config,
                                    const std::vector<std::string>& tokens,
                                    size_t& index) {
    // server の後は '{' が来るはず
    if (HttpConfigParser::getNextToken(tokens, index) != BRACE_OPEN) {
        throw std::runtime_error("Error: Expected '{' after server directive");
    }

    //このサーバーの設定を保持する ServerConfig server_config;
    ServerConfig server_config;
    bool found_closing_brace = false;
    // BRACE_CLOSE が来るまでループ
    while (!HttpConfigParser::isEof(tokens, index)) {
        std::string token = HttpConfigParser::getNextToken(tokens, index);
        if (token == BRACE_CLOSE) {  // BRACE_CLOSE=="}"
            // server ブロック終了
            found_closing_brace = true;
            break;
        }

        if (token ==
            KEYWORD_LOCATION) {  // KEYWORD_LOCATION=="location"を見つけたら、locationを呼び出す。
            HttpConfigParser::parserLocation(server_config, tokens, index);
        } else if (token == DIRECTIVE_LISTEN) {  // DIRECTIVE_LISTEN=="listen"
            ListenDirective ld = HttpConfigParser::parseListen(tokens, index);
            server_config.setListen(ld);
        } else if (HttpConfigParser::parseCommonDirective(server_config, token,
                                                          tokens, index)) {
            continue;
        } else {
            //知らないディレクティブはエラー
            throw std::runtime_error(
                "Error: Unknown directive in server block: " + token);
        }
    }
    if (!found_closing_brace) {
        throw std::runtime_error("Error: Expected '}' to close server block");
    }

    //完成した server_config を config に追加
    config.addServerConfig(server_config);
}

//-----------------------------------------------------------
//------------------ロケーションブロックパーサー----------------
//-----------------------------------------------------------

///@brief location ブロックをパースする
///@param server_config ServerConfig オブジェクト (参照渡し)
///@param tokens トークンリスト
///@param index 現在の位置 (参照渡しで、この値が内部で進められる)

void HttpConfigParser::parserLocation(ServerConfig& server_config,
                                      const std::vector<std::string>& tokens,
                                      size_t& index) {
    //"location"の後にパスが来るはず
    // 次は、"パス" (例: "/" や "/images")
    std::string path = HttpConfigParser::getNextToken(tokens, index);

    //その次に '{' が来るはず
    if (HttpConfigParser::getNextToken(tokens, index) != BRACE_OPEN) {
        throw std::runtime_error("Error: Expected '{' after location path: " +
                                 path);
    }

    //このロケーションの設定を保持する LocationConfig location_config(path);
    LocationConfig location_config;

    location_config.setPath(path);

    bool found_closing_brace = false;
    // BRACE_CLOSE が来るまでループ
    while (!HttpConfigParser::isEof(tokens, index)) {
        std::string token = HttpConfigParser::getNextToken(tokens, index);
        if (token == BRACE_CLOSE) {  //"}"
            // location ブロック終了
            found_closing_brace = true;
            break;
        }
        if (HttpConfigParser::parseCommonDirective(location_config, token,
                                                   tokens, index)) {
            continue;
        }
        //知らないディレクティブはエラー
        throw std::runtime_error(
            "Error: Unknown directive in location block: " + token);
    }
    if (!found_closing_brace) {
        // ループを抜けたのに閉じ括弧が見つからなかった場合
        throw std::runtime_error(
            "Error: location block missing closing brace '}'");
    }
    //完成した location_config を、引数の server_config に追加する
    server_config.addLocation(location_config);
}

//-----------------------------------------------------------------
//------------------rootディレクティブパーサー---------------------
//-----------------------------------------------------------------
///@brief root ディレクティブをパースする
//@return root のパス　"/var/www/html"など
std::string HttpConfigParser::parseRoot(const std::vector<std::string>& tokens,
                                        size_t& index) {
    // root の後にパスが来るはず
    std::string path = HttpConfigParser::getNextToken(tokens, index);
    //セミコロンを期待
    if (HttpConfigParser::getNextToken(tokens, index) != SEMICOLON) {  //";"
        throw std::runtime_error("Error: Expected ';' after root directive");
    }
    return path;
}

//-----------------------------------------------------------------
//------------------autoindexディレクティブパーサー-----------------
//-----------------------------------------------------------------

///@brief "autoindex" ディレクティブをパースする
//@return autoindex のオンオフ(true/false)
bool HttpConfigParser::parseAutoindex(const std::vector<std::string>& tokens,
                                      size_t& index) {
    // autoindex の後に "on" か "off"が来るはず
    std::string value = HttpConfigParser::getNextToken(tokens, index);

    if (HttpConfigParser::getNextToken(tokens, index) != SEMICOLON) {  //";"
        throw std::runtime_error(
            "Error: Expected ';' after autoindex directive");
    }
    if (value == VALUE_ON) {
        return true;
    }
    if (value == VALUE_OFF) {
        return false;
    }
    throw std::runtime_error("Error: autoindex value must be 'on' or 'off'");
}

//-----------------------------------------------------------------
//------------------indexディレクティブパーサー---------------------
//-----------------------------------------------------------------

///@brief index ディレクティブをパースする (複数形対応)
//@return index ファイル名のリスト""index.html"など
std::vector<std::string> HttpConfigParser::parseIndex(
    const std::vector<std::string>& tokens, size_t& index) {
    std::vector<std::string> index_files;

    // SEMICOLON が来るまでループ
    while (!HttpConfigParser::isEof(tokens, index)) {
        std::string token = HttpConfigParser::getNextToken(tokens, index);
        if (token == SEMICOLON) {  //";"
            // index ディレクティブ終了
            break;
        }
        index_files.push_back(token);
    }

    // breakでループを抜けた場合、getNextTokenがSEMICOLONを消費しているので問題なし
    //  もしEOFに達した場合はエラー

    //１つもファイル名がない場合はエラー
    if (index_files.empty()) {
        throw std::runtime_error(
            "Error: Expected at least one index file before ';'");
    }

    return index_files;
}

//-----------------------------------------------------------------
//------------------listenディレクティブパーサー---------------------
//-----------------------------------------------------------------

//------------------ポート検証ヘルパー関数-------------------------
uint16_t HttpConfigParser::validateAndConvertPort(
    uint64_t temp_port, const std::string& error_value) {
    // クラスのメンバーなので、 `MAX_PORT_NUMBER` に直接アクセスできる
    if (temp_port > MAX_PORT_NUMBER) {
        throw std::runtime_error("Error: Port number out of range: " +
                                 error_value);
    }
    return static_cast<uint16_t>(temp_port);
}

///@brief listen ディレクティブをパースする
///@return 完成した ListenDirective オブジェクト
ListenDirective HttpConfigParser::parseListen(
    const std::vector<std::string>& tokens, size_t& index) {
    ListenDirective ld;

    std::string value = HttpConfigParser::getNextToken(tokens, index);
    std::string::size_type colon_pos = value.find(':');

    uint64_t temp_port;

    if (colon_pos != std::string::npos) {
        // address:port 形式
        ld.address = value.substr(0, colon_pos);
        std::string port_str = value.substr(colon_pos + 1);

        std::stringstream ss(port_str);
        if (!(ss >> temp_port) || !ss.eof()) {
            throw std::runtime_error("Error: Invalid port number in: " +
                                     port_str);
        }

        ld.port = HttpConfigParser::validateAndConvertPort(temp_port, port_str);

    } else {
        // port 形式のみ
        ld.address = DEFAULT_ADDRESS;
        std::stringstream ss(value);

        if (!(ss >> temp_port) || !ss.eof()) {
            throw std::runtime_error("Error: Invalid port number: " + value);
        }
        ld.port = HttpConfigParser::validateAndConvertPort(temp_port, value);
    }

    std::string next_token = HttpConfigParser::getNextToken(tokens, index);
    if (next_token == KEYWORD_DEFAULT_SERVER) {
        ld.is_default_server = true;
        next_token = HttpConfigParser::getNextToken(tokens, index);
    }

    if (next_token != SEMICOLON) {
        throw std::runtime_error("Error: Expected ';' after listen directive");
    }

    return ld;
}

//-----------------------------------------------------------------
//------------------ClientMaxBodySizeディレクティブパーサー----------
//-----------------------------------------------------------------
///@brief client_max_body_size ディレクティブをパースする
///@return ボディサイズの上限値
off_t HttpConfigParser::parseClientMaxBodySize(
    const std::vector<std::string>& tokens, size_t& index) {
    //値のトークンを取得(10mなど)
    std::string value = HttpConfigParser::getNextToken(tokens, index);

    //セミコロンを期待
    if (HttpConfigParser::getNextToken(tokens, index) != SEMICOLON) {  //";"
        throw std::runtime_error(
            "Error: Expected ';' after client_max_body_size directive");
    }

    //値を解析(数値部分と単位部分に分けるk, m, g)
    off_t size = 0;
    std::string num_part;
    char suffix = 0;

    //数値部分とサフィックス部分に分ける
    size_t i = 0;
    while (i < value.size() && std::isdigit(value[i])) {
        i++;
    }
    num_part = value.substr(0, i);

    if (i < value.size()) {
        suffix = static_cast<char>(std::tolower(value[i]));
        if (i + 1 != value.size() ||
            (suffix != SUFFIX_KILOBYTE && suffix != SUFFIX_MEGABYTE &&
             suffix != SUFFIX_GIGABYTE)) {
            throw std::runtime_error(
                "Error: Invalid client_max_body_size value: " + value);
        }
    }

    //数値部分をoff_tに変換
    std::stringstream ss(num_part);
    if (!(ss >> size) || !ss.eof() || size < 0) {
        throw std::runtime_error(
            "Error: Invalid number in client_max_body_size: " + num_part);
    }

    // サフィックスに基づいて乗算
    //    (オーバーフローチェック。off_t の最大値 / 1024
    //    より大きい数に乗算すると危険)
    const off_t max_off_t = std::numeric_limits<off_t>::max();

    if (suffix == SUFFIX_KILOBYTE) {
        // オーバーフローチェック: size > MAX / 1024
        if (size > max_off_t / BYTES_PER_KB) {
            throw std::runtime_error(
                "Error: client_max_body_size (k) overflows");
        }
        size *= BYTES_PER_KB;
    } else if (suffix == SUFFIX_MEGABYTE) {
        // オーバーフローチェック: size > MAX / (1024*1024)
        if (size > max_off_t / BYTES_PER_MB) {
            throw std::runtime_error(
                "Error: client_max_body_size (m) overflows");
        }
        size *= BYTES_PER_MB;
    } else if (suffix == SUFFIX_GIGABYTE) {
        // オーバーフローチェック: size > MAX / (1024*1024*1024)
        if (size > max_off_t / BYTES_PER_GB) {
            throw std::runtime_error(
                "Error: client_max_body_size (g) overflows");
        }
        size *= BYTES_PER_GB;
    }

    return size;
}

//-----------------------------------------------------------------
//------------------error_pageディレクティブパーサー---------------
//-----------------------------------------------------------------
///@brief error_page ディレクティブをパースする
///       (例: error_page 404 500 /50x.html;)
///       (例: error_page 403 =200 /index.html;)
HttpConfigParser::ParsedErrorPage HttpConfigParser::parseErrorPage(
    const std::vector<std::string>& tokens, size_t& index) {
    ParsedErrorPage pep;
    std::string token;

    // ステータスコードを読み込む (数値が続く限り)
    while (!isEof(tokens, index)) {
        token = getNextToken(tokens, index);

        // トークンが数値 (ステータスコード) かどうかをチェック
        std::stringstream ss(token);
        int status_code;
        // (ss >> status_code) で変換を試み、
        // ss.eof() で "404foo" のような余計な文字がないことを確認
        // 課題の要件ではエラーページは 300-599 の範囲が妥当
        if ((ss >> status_code) && ss.eof() &&
            status_code >= MIN_ERROR_STATUS_CODE &&
            status_code <= MAX_ERROR_STATUS_CODE) {
            pep.status_codes.push_back(status_code);
        } else {
            // 数値でなければ、ループを抜ける
            // この 'token' は、'=' か ターゲットパス (e.g., "/50x.html") のはず
            break;
        }
    }

    // ループがEOFで抜けた場合、ターゲットURIや'='がない
    if (isEof(tokens, index) && !pep.status_codes.empty()) {
        // (status_codesが空の場合は、次のempty()チェックでエラーになるのでここではじかない)
        throw std::runtime_error(
            "Error: Expected target URI or '=' after status code(s)");
    }

    if (pep.status_codes.empty()) {
        throw std::runtime_error(
            "Error: Expected status code(s) for error_page");
    }

    // オプションの '=' (ステータスコード上書き) をチェック
    if (token == "=") {
        if (isEof(tokens, index)) {
            throw std::runtime_error(
                "Error: Expected new status code after '=' in error_page");
        }
        token = getNextToken(tokens, index);

        std::stringstream ss(token);
        if (!(ss >> pep.directive.override_status) || !ss.eof() ||
            pep.directive.override_status < MIN_OVERRIDE_STATUS_CODE ||
            pep.directive.override_status > MAX_ERROR_STATUS_CODE) {
            throw std::runtime_error(
                "Error: Invalid new status code in error_page: " + token);
        }

        // 最後の引数 (ターゲットURI) を取得
        if (isEof(tokens, index)) {
            throw std::runtime_error(
                "Error: Expected target URI after status code in error_page");
        }
        pep.directive.target = getNextToken(tokens, index);
        // URI検証
        if (pep.directive.target.empty() ||
            pep.directive.target[0] !=
                '/') {  //ターゲットURIは'/'で始まらなければならない：500がターゲットに入った×
            throw std::runtime_error(
                "Error: Invalid target URI in error_page, must start with "
                "'/': " +
                pep.directive.target);
        }

    } else {
        // '=' がなかった場合
        pep.directive.override_status = -1;  // -1 を「上書きなし」とする

        // ここで検証を追加
        // ターゲットURIは '/' で始まらなければならない
        if (token.empty() || token[0] != '/') {
            throw std::runtime_error(
                "Error: Invalid target URI in error_page, must start with "
                "'/': " +
                token);
        }
        pep.directive.target = token;  // 検証OK
    }

    //最後にセミコロンがあるか確認
    if (getNextToken(tokens, index) != SEMICOLON) {
        throw std::runtime_error(
            "Error: Expected ';' after error_page directive");
    }

    return pep;
}
