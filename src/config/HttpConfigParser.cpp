#include "HttpConfigParser.h"
#include <cctype> // std::isspace
#include <sstream>//parseListen用
#include <limits> // std::numeric_limits を使うために必要

//構文解析用クラスkeyword
const char* const HttpConfigParser::SPECIAL_CHARS = "{};";
const char HttpConfigParser::HASH_CHAR = '#';
const char* const HttpConfigParser::KEYWORD_HTTP = "http";
const char* const HttpConfigParser::KEYWORD_SERVER = "server";
const char* const HttpConfigParser::KEYWORD_LOCATION = "location";
const char* const HttpConfigParser::BRACE_OPEN = "{";
const char* const HttpConfigParser::BRACE_CLOSE = "}";
const char* const HttpConfigParser::SEMICOLON = ";";
//Directive keywords
const char* const HttpConfigParser::DIRECTIVE_LISTEN = "listen";
const char* const HttpConfigParser::DIRECTIVE_ROOT = "root";
const char* const HttpConfigParser::DIRECTIVE_INDEX = "index";
const char* const HttpConfigParser::DIRECTIVE_AUTOINDEX = "autoindex";
const char* const HttpConfigParser::DIRECTIVE_CLIENT_MAX_BODY_SIZE = "client_max_body_size";

//マジックナンバー定数に
const off_t HttpConfigParser::BYTES_PER_KB = 1024;
const off_t HttpConfigParser::BYTES_PER_MB = 1024 * 1024;
const off_t HttpConfigParser::BYTES_PER_GB = 1024 * 1024 * 1024;


//終端に来たかどうか
bool HttpConfigParser::isEof(const std::vector<std::string>& tokens, size_t index) {
	return index >= tokens.size();
}

//次のトークンを取得し、インデックスを1つ進める
std::string HttpConfigParser::getNextToken(const std::vector<std::string>& tokens, size_t& index) {
	if (isEof(tokens, index)) {
		throw std::runtime_error("Error: Unexpected end of EOF");
	}
	return tokens[index++];
}

//ファイル読み込みとトークン化
std::vector<std::string> HttpConfigParser::tokenize(const std::string& filename){
	std::ifstream ifs(filename.c_str());
	if (!ifs.is_open()) {
		throw std::runtime_error("Error: Could not open file " + filename);
	}

	std::vector<std::string> tokens;
	std::string line;


	while (std::getline(ifs, line)) {
		for (size_t i = 0; i < line.size(); ++i) {
			// 空欄スキップ
			if (std::isspace(line[i])) {continue;}
			// コメント行スキップ
			if (line[i] == HASH_CHAR) {break;}//HASH_CHAR==#
			// トークン抽出
			if (std::string(SPECIAL_CHARS).find(line[i]) != std::string::npos) {
                tokens.push_back(line.substr(i, 1));// SPECIAL_CHARS=="{};"//1文字切り出す
                continue;
            }
			size_t start = i;
			while(i < line.size() && 
                  !std::isspace(line[i]) &&
                  line[i] != HASH_CHAR &&
                  std::string(SPECIAL_CHARS).find(line[i]) == std::string::npos)
            {
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

HttpConfig HttpConfigParser::parse(const std::string& filename){
	std::vector<std::string> tokens = HttpConfigParser::tokenize(filename);

	//parse処理に必要な道具をローカルに準備
	HttpConfig config;//完成させるHttpConfigオブジェクト
	size_t index = 0;//現在のトークン位置

	//最初のトークンが "http" であることを確認
	if (isEof(tokens, index) || getNextToken(tokens, index) != KEYWORD_HTTP) {
        throw std::runtime_error("Error: Expected 'http' block at the beginning");
    }

	// 3. 次のトークンが "{" であることを確認
    if (isEof(tokens, index) || getNextToken(tokens, index) != BRACE_OPEN) {
        throw std::runtime_error("Error: Expected '{' after http directive");
    }

	// 4. http ブロックの中身をパース
    CommonConfig http_common_config; // httpレベルの共通設定をここに溜める
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
        }
        else if (token == DIRECTIVE_ROOT) {        // --- ↓↓ http レベルのディレクティブ↓↓ ---
            std::string r = parseRoot(tokens, index);
            http_common_config.setRoot(r);
        }
        else if (token == DIRECTIVE_INDEX) {
            std::vector<std::string> files = parseIndex(tokens, index);
            for (size_t i = 0; i < files.size(); ++i) {
                http_common_config.addIndexFile(files[i]);
            }
        }
        else if (token == DIRECTIVE_AUTOINDEX) {
            bool ai = parseAutoindex(tokens, index);
            http_common_config.setAutoindex(ai);
        }
        else if (token == DIRECTIVE_CLIENT_MAX_BODY_SIZE) {
            off_t size = parseClientMaxBodySize(tokens, index);
            http_common_config.setClientMaxBodySize(size);
        }
        // ★ ステップ5でここに error_page, return のパース処理を追加します
        else {
            throw std::runtime_error("Error: Unknown directive in http block: " + token);
        }
    }

    if (!found_closing_brace) {
        throw std::runtime_error("Error: Expected '}' to close http block");
    }

    // 5. 読み取った http レベルの設定を、config 全体のデフォルトとして保存
    config.setDefaults(http_common_config);


    // ★★★ 次のステップ4（継承処理）でここにコードを追加します ★★★
    // (今はまだ空でOKです)

    return config;
}

//-----------------------------------------------------------
//------------------サーバーブロックパーサー-------------------
//-----------------------------------------------------------

///@brief server ブロックをパースする
///@param config HttpConfig オブジェクト (参照渡し)
///@param tokens トークンリスト
///@param index 現在の位置 (参照渡しで、この値が内部で進められる)

void HttpConfigParser::parserServer(HttpConfig& config, const std::vector<std::string>& tokens, size_t& index) {
	//server の後は '{' が来るはず
	if(HttpConfigParser::getNextToken(tokens, index) != BRACE_OPEN) {
		throw std::runtime_error("Error: Expected '{' after server directive");
	}

	//このサーバーの設定を保持する ServerConfig server_config;
	ServerConfig server_config;
	bool found_closing_brace = false;
	//BRACE_CLOSE が来るまでループ
	while(!HttpConfigParser::isEof(tokens, index)) {
		std::string token = HttpConfigParser::getNextToken(tokens, index);
		if (token == BRACE_CLOSE) {//BRACE_CLOSE=="}"
			//server ブロック終了
			found_closing_brace = true;
			break;
		}
		
		if(token == KEYWORD_LOCATION) {//KEYWORD_LOCATION=="location"
			//"location"を見つけたら、locationを呼び出す。
			HttpConfigParser::parserLocation(server_config, tokens, index);
		}
		else if (token == DIRECTIVE_LISTEN) {//DIRECTIVE_LISTEN=="listen"
			// listen を呼び出し、結果をserver_configにセット
			ListenDirective ld = HttpConfigParser::parseListen(tokens, index);
			server_config.setListen(ld); //<- セッターが必要
		}
		else if (token == DIRECTIVE_ROOT) {//DIRECTIVE_ROOT== "root"
			// root を呼び出し、結果をserver_configにセット
			std::string r = HttpConfigParser::parseRoot(tokens, index);
			server_config.setRoot(r); //<- セッターが必要
		}
		else if (token == DIRECTIVE_INDEX) {//DIRECTIVE_INDEX=="index"
			// index を呼び出し、結果をserver_configにセット
			std::vector<std::string> files = HttpConfigParser::parseIndex(tokens, index);
			for (size_t i = 0; i < files.size(); ++i) {
				server_config.addIndexFile(files[i]); //<- セッターが必要
			}
		}
		else if (token == DIRECTIVE_AUTOINDEX) {//DIRECTIVE_AUTOINDEX==autoindex
			// parseAutoindex 呼び出し、結果をserver_configにセット
			bool ai = HttpConfigParser::parseAutoindex(tokens, index);
			server_config.setAutoindex(ai); //<- セッターが必要
		}
		else if (token == DIRECTIVE_CLIENT_MAX_BODY_SIZE) {//DIRECTIVE_CLIENT_MAX_BODY_SIZE == "autoindex"
            off_t size = HttpConfigParser::parseClientMaxBodySize(tokens, index);
            server_config.setClientMaxBodySize(size); // (CommonConfig::setClientMaxBodySize セッター)
        }
		// error_page, return,などのディレクティブのパース処理を追加予定???
		else {
			//知らないディレクティブはエラー
			throw std::runtime_error("Error: Unknown directive in server block:" + token);
		}
	}
	if (!found_closing_brace) {
		throw std::runtime_error("Error: Expected '}' to close server block");
		
	}

	//完成した server_config を config に追加//// (※ HttpConfig.h に public な addServer(ServerConfig s) セッターが必要)
	config.addServerConfig(server_config);
}


//-----------------------------------------------------------
//------------------ロケーションブロックパーサー----------------
//-----------------------------------------------------------

///@brief location ブロックをパースする
///@param server_config ServerConfig オブジェクト (参照渡し)
///@param tokens トークンリスト
///@param index 現在の位置 (参照渡しで、この値が内部で進められる)

void HttpConfigParser::parserLocation(ServerConfig& server_config, const std::vector<std::string>& tokens, size_t& index){

	//"location"の後にパスが来るはず
	// 次は、"パス" (例: "/" や "/images")
	std::string path = HttpConfigParser::getNextToken(tokens, index);
	
	//その次に '{' が来るはず
	if(HttpConfigParser::getNextToken(tokens, index) != BRACE_OPEN) {
		throw std::runtime_error("Error: Expected '{' after location path: " + path);
	}

	//このロケーションの設定を保持する LocationConfig location_config(path);
	LocationConfig location_config;

	// (※ LocationConfig.h に public な setPath(std::string s) セッターが必要です)
	location_config.setPath(path);

	bool found_closing_brace = false; // ⇐追加copilot
	//BRACE_CLOSE が来るまでループ
	while(!HttpConfigParser::isEof(tokens, index)) {
		std::string token = HttpConfigParser::getNextToken(tokens, index);
		if (token == BRACE_CLOSE) {//"}"
			//location ブロック終了
			found_closing_brace = true;
			break;
		}
		if (token == DIRECTIVE_ROOT) {
			std::string r = HttpConfigParser::parseRoot(tokens, index);
			location_config.setRoot(r); //<- セッターが必要
		}
		else if (token == DIRECTIVE_INDEX) {
			std::vector<std::string> files = HttpConfigParser::parseIndex(tokens, index);
			for (size_t i = 0; i < files.size(); ++i) {
				location_config.addIndexFile(files[i]); //<- セッターが必要
			}
		}
		else if (token == DIRECTIVE_AUTOINDEX) {
			bool ai = HttpConfigParser::parseAutoindex(tokens, index);
			location_config.setAutoindex(ai); //<- セッターが必要
		}
		else if (token == DIRECTIVE_CLIENT_MAX_BODY_SIZE) {
            off_t size = HttpConfigParser::parseClientMaxBodySize(tokens, index);
            location_config.setClientMaxBodySize(size); // (CommonConfig::setClientMaxBodySize セッター)
        }
		// error_page, return,などのディレクティブのパース処理を追加予定???
		else {
			//知らないディレクティブはエラー
			throw std::runtime_error("Error: unknown directive in location block: " + token);
		}
	}
	if (!found_closing_brace) {
		// ループを抜けたのに閉じ括弧が見つからなかった場合
		throw std::runtime_error("Error: location block missing closing brace '}'");
	}
		// 6. 完成した location_config を、引数の server_config に追加する
    // (※ ServerConfig.h に public な addLocation(LocationConfig l) セッターが必要です)
    server_config.addLocation(location_config);
}


//-----------------------------------------------------------------
//------------------rootディレクティブパーサー---------------------
//-----------------------------------------------------------------
///@brief root ディレクティブをパースする
//@return root のパス　"/var/www/html"など
std::string HttpConfigParser::parseRoot(const std::vector<std::string>& tokens, size_t& index) {
	//root の後にパスが来るはず
	std::string path = HttpConfigParser::getNextToken(tokens, index);
	//セミコロンを期待
	if (HttpConfigParser::getNextToken(tokens, index) != SEMICOLON) {//";"
		throw std::runtime_error("Error: Expected ';' after root directive");
	}
	return path;
}

//-----------------------------------------------------------------
//------------------autoindexディレクティブパーサー-----------------
//-----------------------------------------------------------------

///@brief "autoindex" ディレクティブをパースする
//@return autoindex のオンオフ(true/false)
bool HttpConfigParser::parseAutoindex(const std::vector<std::string>& tokens, size_t& index) {
	//autoindex の後に "on" か "off"が来るはず
	std::string value = HttpConfigParser::getNextToken(tokens, index);
	
	if(HttpConfigParser::getNextToken(tokens, index) != SEMICOLON) {//";"
		throw std::runtime_error("Error: Expected ';' after autoindex directive");
	}
	if(value == "on") {
		return true;
	}
	if (value == "off") {
		return false;
	}
	throw std::runtime_error("Error: autoindex value must be 'on' or 'off'");
}

//-----------------------------------------------------------------
//------------------indexディレクティブパーサー---------------------
//-----------------------------------------------------------------

///@brief index ディレクティブをパースする (複数形対応)
//@return index ファイル名のリスト""index.html"など
std::vector<std::string> HttpConfigParser::parseIndex(const std::vector<std::string>& tokens, size_t& index) {
	std::vector<std::string> index_files;

	// SEMICOLON が来るまでループ
	while(!HttpConfigParser::isEof(tokens, index)) {
		std::string token = HttpConfigParser::getNextToken(tokens, index);
		if (token == SEMICOLON) {//";"
			//index ディレクティブ終了
			break;
		}
		index_files.push_back(token);
	}

	//breakでループを抜けた場合、getNextTokenがSEMICOLONを消費しているので問題なし
	// もしEOFに達した場合はエラー

	//１つもファイル名がない場合はエラー
	if (index_files.empty()) {
		throw std::runtime_error("Error: Expected at least one index file before ';'");
	}

	return index_files;
}


//-----------------------------------------------------------------
//------------------listenディレクティブパーサー---------------------
//-----------------------------------------------------------------

///@brief listen ディレクティブをパースする
///@return 完成した ListenDirective オブジェクト
ListenDirective HttpConfigParser::parseListen(const std::vector<std::string>& tokens, size_t& index) {
	ListenDirective ld; //返すための構造体

	// listen の次のトークン取得
	std::string value = HttpConfigParser::getNextToken(tokens, index);

	//本当はlocalhostやドメイン名も解決したいが、とりあえずIPアドレスかポート番号だけ対応★

	// 形式を簡易的にチェック(address:port か port)
	std::string::size_type colon_pos = value.find(':');

	if(colon_pos != std::string::npos) {
		// address:port 形式
		ld.address = value.substr(0, colon_pos);
		std::string port_str = value.substr(colon_pos + 1);

		//C++98で文字列を数値に変換(sstreamを使用)
		std::stringstream ss(port_str);
		if(!(ss >> ld.port) || !ss.eof()) {//変換失敗or あとにごみがある場合
			throw std::runtime_error("Error: Invalid port number in" + port_str);
		}
	} else {
		// port 形式のみ
		ld.address = DEFAULT_ADDRESS; //デフォルトのアドレス

		std::stringstream ss(value);
		if(!(ss >> ld.port) || !ss.eof()) {
			throw std::runtime_error("Error: Invalid port number +" + value);
		}
	}

	// 次のトークンを確認して、"default_server" かどうかチェック
	std::string next_token = HttpConfigParser::getNextToken(tokens, index);
	if (next_token == "default_server") {
		ld.is_default_server = true;
		//さらにもう一つトークンをよみ、セミコロンを期待
		next_token = HttpConfigParser::getNextToken(tokens, index);
	}


	//最後はセミコロンを期待
	if (next_token != SEMICOLON) {//";"
		throw std::runtime_error("Error: Expected ';' after listen directive");
	}

	return ld;
}


//-----------------------------------------------------------------
//------------------ClientMaxBodySizeディレクティブパーサー----------
//-----------------------------------------------------------------
///@brief client_max_body_size ディレクティブをパースする
///@return ボディサイズの上限値
off_t HttpConfigParser::parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t& index) {
	//値のトークンを取得(10mなど)
	std::string value = HttpConfigParser::getNextToken(tokens, index);

	//セミコロンを期待
	if (HttpConfigParser::getNextToken(tokens, index) != SEMICOLON) {//";"
		throw std::runtime_error("Error: Expected ';' after client_max_body_size directive");
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
		suffix = std::tolower(value[i]); //k, m, g のいずれか
		if (i + 1 != value.size() || (suffix != 'k' && suffix != 'm' && suffix != 'g')) {
			throw std::runtime_error("Error: Invalid client_max_body_size value: " + value);
		}
	}

	//数値部分をoff_tに変換
	std::stringstream ss(num_part);
    if (!(ss >> size) || !ss.eof() || size < 0) {
        throw std::runtime_error("Error: Invalid number in client_max_body_size: " + num_part);
    }

	// 5. サフィックスに基づいて乗算
    //    (オーバーフローチェック。off_t の最大値 / 1024 より大きい数に乗算すると危険)
    const off_t max_off_t = std::numeric_limits<off_t>::max();

	if (suffix == 'k') {
        // オーバーフローチェック: size > MAX / 1024
        if (size > max_off_t / BYTES_PER_KB) { 
            throw std::runtime_error("Error: client_max_body_size (k) overflows");
        }
        size *= BYTES_PER_KB;
    } else if (suffix == 'm') {
        // オーバーフローチェック: size > MAX / (1024*1024)
        if (size > max_off_t / BYTES_PER_MB) { 
            throw std::runtime_error("Error: client_max_body_size (m) overflows");
        }
        size *= BYTES_PER_MB;
    } else if (suffix == 'g') {
         // オーバーフローチェック: size > MAX / (1024*1024*1024)
         if (size > max_off_t / BYTES_PER_GB) { 
            throw std::runtime_error("Error: client_max_body_size (g) overflows");
        }
        size *= BYTES_PER_GB;
    }

	return size;
}