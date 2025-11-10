#pragma once

#include <cstddef>
// #include <cstdint>//-std=c++98ではサポートされていない可能性があるため使用できない？？変更
#include "common.h"

// GET・HEAD 以外は GET に変換される
typedef struct ErrorPageDirective {
    // error_page 404 /404.html;
    // error_page 404 =200 のとき、内部転送しステータスコードを上書きする
    std::vector<int> statuses;
    std::string target;
    int override_status;
    ErrorPageDirective(): override_status(-1) {}
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
    // uint16_t port;↓変更
    unsigned short port; //<- uint16_t が c++98 だと標準でサポートされるか怪しいため、unsigned short ポート番号(0-65535)を扱う型　変更
    bool is_default_server;
    ListenDirective() : port(DEFAULT_PORT), is_default_server(false) {}
} ListenDirective;

class CommonConfig {

    public:
    CommonConfig() : client_max_body_size(0), autoindex(false) {}//<- 変更privateからpublicへ

    // --- ↓↓　セッターを追加 ↓↓ ---
    void setClientMaxBodySize(off_t size) { client_max_body_size = size; }
    void setErrorPage(const ErrorPageDirective& ep) { error_page = ep; }
    void setRedirect(const ReturnDirective& ret) { redirect = ret; }
    void setRoot(const std::string& path) { root = path; }
    void setAutoindex(bool on) { autoindex = on; }
    void addIndexFile(const std::string& file) { index_files.push_back(file); }
    void setUploadStore(const std::string& store_path) { upload_store = store_path; }
    // --- 変更 ---

    private:
        // 値が 0 の場合制限なしを意味する
        off_t client_max_body_size;
        ErrorPageDirective error_page;
        ReturnDirective redirect;
        std::string root;
        // 初期化子リストで初期化
        bool autoindex;
        std::vector<std::string> index_files;
        std::string upload_store;

};

class LocationConfig : public CommonConfig {
    
    public:
    // --- ↓↓セッターを追加 ↓↓ ---
    void setPath(const std::string& p) {
        path = p;
    }

    void addAllowedMethod(const Method& m) { allowed_methods.push_back(m); }
    void setCgiPath(const std::string& p) { cgi_path = p; }
    void setCgiExtension(const std::string& e) { cgi_extension = e; }
    //=-- 変更 ---↑↑

    private:
        // 許可するメソッドが入るだけ（例：GET HEAD POST DELETE）
        std::vector<Method> allowed_methods;
        std::string cgi_path;
        std::string cgi_extension;
        std::string path;
};

class ServerConfig : public CommonConfig {

    public:
    // --- ↓↓ セッターを追加 ↓↓ ---
    void addLocation(const LocationConfig& l) {
        locations.push_back(l);
    }

    // --- ↓↓ セッターを追加 ↓↓ ---
    void setListen(const ListenDirective& l) { listens = l; }
    void addServerName(const std::string& name) { server_names.push_back(name); }
    // --- ↑↑ ここまで 変更---

    private:
        std::vector<LocationConfig> locations;
        ListenDirective listens;
        std::vector<std::string> server_names;
};


class HttpConfig {

    public:
    // --- ↓↓ セッターを追加 ↓↓ ---
    // (パーサーが addServerConfig と呼んでいるため名前を合わせる)
    void addServerConfig(const ServerConfig& s) {
        servers.push_back(s);
    }

    // --- セッターを追加 ↓↓ ---
    void setDefaults(const CommonConfig& c) { defaults = c; }
    // --- ↑↑ ここまで変更---

    private:
        std::vector<ServerConfig> servers;
        CommonConfig defaults;
};