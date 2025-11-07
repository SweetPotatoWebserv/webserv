#pragma once

#include <cstddef>
#include <vector>

#include "../core/Common.h"

// GET・HEAD 以外は GET に変換される
typedef struct ErrorPageDirective {
    // error_page 404 /404.html;
    // error_page 404 =200 のとき、内部転送しステータスコードを上書きする
    std::vector<int> statuses;
    std::string target;
    int override_status;
    ErrorPageDirective() : override_status(-1) {}
} ErrorPageDirective;

typedef struct ReturnDirective {
    // return 200;
    // return 200 text;
    // return 200 https://google.com/
    // return 200 /;
    int status;
    std::string text;
    std::string target;
} ReturnDirective;

typedef struct ListenDirective {
    // listen 127.0.0.1:8080;
    // listen 80;
    // listen 127.0.0.1:8080 default_server;
    // listen localhost:8080;
    std::string address;
    // port range: 0 ~ 65535
    uint16_t port;
    bool is_default_server;
    ListenDirective() : port(DEFAULT_PORT), is_default_server(false) {}
} ListenDirective;

class CommonConfig {
   public:
    // 値が 0 の場合制限なしを意味する
    off_t client_max_body_size_;
    ErrorPageDirective error_page_;
    ReturnDirective redirect_;
    std::string root_;
    // 初期化子リストで初期化
    bool autoindex_;
    std::vector<std::string> index_files_;
    std::string upload_store_;

    CommonConfig() : client_max_body_size_(0), autoindex_(false) {}
};

class LocationConfig {
   public:
    CommonConfig common_config_;
    // 許可するメソッドが入るだけ（例：GET HEAD POST DELETE）
    std::vector<Method> allowed_methods_;
    std::string cgi_path_;
    std::string cgi_extension_;
    std::string path_;
};

class ServerConfig {
   public:
    CommonConfig common_config_;
    std::vector<LocationConfig> locations_;
    ListenDirective listens_;
    std::vector<std::string> server_names_;
};

class HttpConfig {
   public:
    std::vector<ServerConfig> servers_;
    CommonConfig defaults_;
};
