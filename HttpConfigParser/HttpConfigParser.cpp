#include "HttpConfigParser.hpp"
#include <cctype> // std::isspace

// --- コンストラクタとデストラクタを削除 (または private 実装) ---
HttpConfigParser::HttpConfigParser() {}
HttpConfigParser::~HttpConfigParser() {}


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

//failを読み込み、トークン化する
std::vector<std::string> HttpConfigParser::tokenize(const std::string& filename){
	std::ifstream ifs(filename.c_str());
	if (!ifs.is_open()) {
		throw std::runtime_error("Error: Could not open file " + filename);
	}

	std::vector<std::string> tokens;
	std::string line;
	std::string special_chars = "{};";

	while (std::getline(ifs, line)) {
		for (size_t i = 0; i < line.size(); ++i) {
			// 空欄スキップ
			if (std::isspace(line[i])) {continue;}
			// コメント行スキップ
			if (line[i] == '#') {break;}
			// トークン抽出
			if (special_chars.find(line[i]) != std::string::npos) {
                tokens.push_back(line.substr(i, 1));
                continue;
            }
			size_t start = i;
			while(i < line.size() && 
                  !std::isspace(line[i]) &&
                  line[i] != '#' &&
                  special_chars.find(line[i]) == std::string::npos)
            {
                ++i;
            }
            tokens.push_back(line.substr(start, i - start));
            --i;
        }
	}
	return tokens;	
}

///@brief パースを実行する唯一の public インターフェース
///@param filename 設定ファイルパス
///@return 完成した HttpConfig オブジェクト

HttpConfig HttpConfigParser::parse(const std::string& filename){
	// トークン化 全トークンを取得
	std::vector<std::string> tokens = HttpConfigParser::tokenize(filename);

	//parse処理に必要な道具をローカルに準備
	HttpConfig config;//完成させるHttpConfigオブジェクト
	size_t index = 0;//現在のトークン位置

	//メインの解析ループ
	// indexが終端(tokens.size())に達するまでループ
	while(!HttpConfigParser::isEof(tokens, index)) {
		//tokensを一つ取り出して、indexを進める
		std::string token = HttpConfigParser::getNextToken(tokens, index);

		//serverブロック以外はエラー
		if (token == "server") {
			HttpConfigParser::parserServer(config, tokens, index);
		} else {
			throw std::runtime_error("Error: Unknown directive outside server block:" + token);
		}
	}
	
	return config;

}


///@brief server ブロックをパースする
///@param config HttpConfig オブジェクト (参照渡し)
///@param tokens トークンリスト
///@param index 現在の位置 (参照渡しで、この値が内部で進められる)

void HttpConfigParser::parserServer(HttpConfig& config, const std::vector<std::string>& tokens, size_t& index) {
	//server の後は '{' が来るはず
	if(HttpConfigParser::getNextToken(tokens, index) != "{") {
		throw std::runtime_error("Error: Expected '{' after server directive");
	}

	//このサーバーの設定を保持する ServerConfig server_config;
	ServerConfig server_config;

	//"}" が来るまでループ
	while(!HttpConfigParser::isEof(tokens, index)) {
		std::string token = HttpConfigParser::getNextToken(tokens, index);
		if (token == "}") {
			//server ブロック終了
			break;
		}

		//ここに"listen"や"root"などのパース処理を追加していく
		else {
			//知らないディレクティブはセミコロンまでスキップ
			while (HttpConfigParser::getNextToken(tokens, index) != ";") {
				//;を見つけるまで進める
				if (HttpConfigParser::isEof(tokens, index)) {
					throw std::runtime_error("Error: Expected ';' to terminate directive");
				}
				
			}
		}
	}

	//完成した server_config を config に追加//// (※ HttpConfig.h に public な addServer(ServerConfig s) セッターが必要)
	config.addServerConfig(server_config);
}