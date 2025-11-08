#pragma once

#include <string>
#include <vector>
#include <fstream>//file read
#include <sstream>//file read
#include <stdexcept>//exception
#include "HttpConfig.h"

class HttpConfigParser {
	public:
		HttpConfigParser(const std::string& filename);//constructor
		~HttpConfigParser();//destructor

		HttpConfig getConfig();//完成したHttpConfigを返す

		const std::vector<std::string>& getTokens() const { return tokens_; }//トークン列を返すtest用
	#include <iostream> // デバッグ用
	void printTokens() {
		for (size_t i = 0; i < tokens_.size(); ++i) {
			std::cout << "[" << tokens_[i] << "]" << std::endl;
		}
	}

	private:
		HttpConfig config_;//パース結果を格納する
		std::vector<std::string> tokens_;//トークン列を格納する
		size_t current_token_index_;//現在のトークン位置を示すインデックス

		void loadAndTokenize();//ファイルを読み込み、トークンに分割する

		void tokenize(const std::string& line);//1行をトークンに分割する

		void parseConfig();//トークン列をパースしてHttpConfigを構築する
};