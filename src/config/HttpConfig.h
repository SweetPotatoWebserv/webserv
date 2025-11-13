#pragma once

#include <cstddef>
// #include <cstdint>//-std=c++98ではサポートされていない可能性があるため使用できない？？変更
#include "../core/Common.h" // DEFAULT_PORT


// GET・HEAD 以外は GET に変換される
typedef struct ErrorPageDirective {
    // error_page 404 /404.html;
    // error_page 404 =200 のとき、内部転送しステータスコードを上書きする
    std::string target;
    int override_status;
    ErrorPageDirective();
    ErrorPageDirective(std::string t, int o);
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

    uint16_t port;//<-修正
    bool is_default_server;
    ListenDirective();
} ListenDirective;

class CommonConfig {

    public:
    CommonConfig()
        : client_max_body_size(-1),  // ０は制限なしを意味するので-1に初期化
          autoindex(false),          // デフォルトは off
          autoindex_is_set(false),  // autoindex が設定されたかどうかのフラグ
          root_is_set(false),   // root が設定されたかどうかのフラグ
          upload_store_is_set(false), // upload_store が設定されたかどうかのフラグ
          redirect_is_set(false)
    {}

    // --- ↓↓　セッターを追加 ↓↓ ---
    void setRoot(const std::string& path) {
        root = path;
        root_is_set = true;// root が設定されたことを記録
    }
    void setAutoindex(bool on) {
        autoindex = on;
        autoindex_is_set = true;
    }

    void setUploadStore(const std::string& store_path) { 
        upload_store = store_path;
        upload_store_is_set = true;
     }

     void setRedirect(const ReturnDirective& ret) {
        redirect = ret;
        redirect_is_set = true;
    }

    void addErrorPage(int status, const ErrorPageDirective& ep) {
        error_page[status] = ep; 
    }
    void setClientMaxBodySize(off_t size) { client_max_body_size = size; }
    void addIndexFile(const std::string& file) { index_files.push_back(file); }
    
    // --- 変更 ---


    bool isRootSet() const {return root_is_set; }
    const std::string& getRoot() const { return root; }

    bool isAutoindexSet() const { return autoindex_is_set; }
    bool getAutoindex() const { return autoindex; }

    bool isUploadStoreSet() const { return upload_store_is_set; }
    const std::string& getUploadStore() const { return upload_store; }
    
    bool isRedirectSet() const { return redirect_is_set; }
    const ReturnDirective& getRedirect() const { return redirect; }

    off_t getClientMaxBodySize() const { return client_max_body_size; }
    const std::map<int, ErrorPageDirective>& getErrorPages() const { return error_page; }
    const std::vector<std::string>& getIndexFiles() const { return index_files; }

    private:
        bool root_is_set;
        std::string root;

        bool autoindex_is_set;
        bool autoindex;

        bool upload_store_is_set;//private管理で
        std::string upload_store;

        ReturnDirective redirect;
        bool redirect_is_set;

        // 値が 0 の場合制限なしを意味する
        off_t client_max_body_size;
        std::map<int, ErrorPageDirective> error_page;
        std::vector<std::string> index_files;

};

class LocationConfig {
    
    public:
    // --- ↓↓セッターを追加 ↓↓ ---
    void setPath(const std::string& p) {
        path = p;
    }
        void setRoot(const std::string& r) { common_config.setRoot(r); }
		void addIndexFile(const std::string& file) { common_config.addIndexFile(file); }
		void setAutoindex(bool on) { common_config.setAutoindex(on); }
        void setClientMaxBodySize(off_t size) { common_config.setClientMaxBodySize(size); }

    void addAllowedMethod(const Method& m) { allowed_methods.push_back(m); }
    void setCgiPath(const std::string& p) { cgi_path = p; }
    void setCgiExtension(const std::string& e) { cgi_extension = e; }
    //=-- 変更 ---↑↑

    private:
    CommonConfig common_config; // 共通設定
        // 許可するメソッドが入るだけ（例：GET HEAD POST DELETE）
        std::vector<Method> allowed_methods;
        std::string cgi_path;
        std::string cgi_extension;
        std::string path;
};

class ServerConfig {

    public:
    // --- ↓↓ セッターを追加 ↓↓ ---
    void addLocation(const LocationConfig& l) {
        locations.push_back(l);
    }

    // --- ↓↓ セッターを追加 ↓↓ ---
    void setListen(const ListenDirective& l) { listens = l; }
    void addServerName(const std::string& name) { server_names.push_back(name); }
    // --- ↑↑ ここまで 変更---

    void setRoot(const std::string& r) { common_config.setRoot(r); }
	void addIndexFile(const std::string& file) { common_config.addIndexFile(file); }
	void setAutoindex(bool on) { common_config.setAutoindex(on); }
    void setClientMaxBodySize(off_t size) { common_config.setClientMaxBodySize(size); }

    private:
        std::vector<LocationConfig> locations;
        ListenDirective listens;
        std::vector<std::string> server_names;
        CommonConfig common_config; // 共通設定
};


class HttpConfig {

    public:
    // --- ↓↓ セッターを追加 ↓↓ ---
    // (パーサーが addServerConfig と呼んでいるため名前を合わせる)
    void addServerConfig(const ServerConfig& s) {
        servers.push_back(s);
    }

    // --- セッターを追加 ↓↓ ---
    void setDefaults(const CommonConfig& c) { common_config = c; }
    // --- ↑↑ ここまで変更---

    private:
        std::vector<ServerConfig> servers;
        CommonConfig common_config; // 共通設定
};