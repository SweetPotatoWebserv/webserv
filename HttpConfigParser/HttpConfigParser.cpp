#include "HttpConfigParser.hpp"
#include <cctype> // std::isspace

// コンストラクタ
HttpConfigParser::HttpConfigParser(const std::string& filename) : current_token_index_(0) {
    //1.file open
	std::ifstream ifs(filename.c_str());
	if (!ifs.is_open()) {
		throw std::runtime_error("Failed to open config file: " + filename);
	}
	//2.file全体をstringstreamnに読み込む
	std::stringstream ss;
	ss << ifs.rdbuf();
	//3. stringstreamをstringに変換
	std::string content = ss.str();
	//4. トークナイズを呼び出す
	this ->tokenize(content);

    // ここで filename を使ってファイルを読み込み、
    // tokenize() を呼び出す処理を後で書きます。
    (void)filename; // とりあえず「使ってない」警告を消す
}

// デストラクタ
HttpConfigParser::~HttpConfigParser() {
    // 今は何もすることがない
}

// 完成した HttpConfig を返す関数
HttpConfig HttpConfigParser::getConfig() {
    // 後で parse() を呼び出すように変更する
    return this->config_;
}

// --- private メンバ関数 ---

// ファイルを読み込み、トークン化する (Day 1 の核心)
void HttpConfigParser::loadAndTokenize() {
    //ファイルを読み込み、トークンに分割する
}

// トークナイザーの本体
void HttpConfigParser::tokenize(const std::string& content) {
	std::string special_chars = "{};"; //一文字でトークンになる文字

	for (size_t i = 0; i < content.size(); ++i) {
		//1. 空白文字のスキップ
		if(std::isspace(content[i])) {
			continue; //空白文字はスキップ
		}
		//2. コメント行のスキップ
		if (content[i] == '#') {
			//コメント行をスキップ
			while (i < content.size() && content[i] != '\n') {
				++i;
			}
			continue;
		}
		//3. 特殊文字 ( { } ; # ) は1文字でトークン
		if (special_chars.find(content[i]) != std::string::npos) {
			this->tokens_.push_back(content.substr(i, 1));
			continue;
		}

		// 4. 上記以外 (単語: "server", "8080", "/var/www" など)
		size_t start = i;
		while(i < content.size() && //最後の文字まで到達しておらず、かつ
				!std::isspace(content[i]) && //空白文字でなく、かつ
				content[i] != '#' && //コメント文字でなく、かつ
				special_chars.find(content[i]) == std::string::npos)//特殊文字でない間
		{
			++i;
		}

		// トークンを抽出して保存
		// [start] から [i-1] までの部分文字列を切り出す
		this->tokens_.push_back(content.substr(start, i - start));
		--i; //外側の for ループの i++ を相殺
	}

}

// (Day 2 で追加) パースを実行するメイン関数
void HttpConfigParser::parseConfig() {
    //トークン列をパースしてHttpConfigを構築する
}