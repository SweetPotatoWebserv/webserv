#pragma once

#include <sys/types.h>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "../core/Common.h"

// GET・HEAD 以外は GET に変換される
typedef struct ErrorPageDirective {
    // error_page 404 /404.html;
    // error_page 404 =200 のとき、内部転送しステータスコードを上書きする
    std::string target;
    int override_status;
    ErrorPageDirective();
    ErrorPageDirective(const std::string& t, int o);
} ErrorPageDirective;

typedef struct ReturnDirective {
    // return 200;
    // return 200 text;
    // return 200 https://google.com/
    // return 200 /;
    int status;
    std::string text;
    std::string target;
    ReturnDirective();
} ReturnDirective;

typedef struct ListenDirective {
    // listen 127.0.0.1:8080;
    // listen 80;
    // listen 127.0.0.1:8080 default_server;
    // listen localhost:8080;
    std::string address;

    uint16_t port;
    bool is_default_server;
    ListenDirective();
} ListenDirective;

typedef struct AutoIndexDirective {
    bool is_set_;
    bool value_;
    AutoIndexDirective();
} AutoIndexDirective;

typedef struct RootDirective {
    bool is_set_;
    std::string value_;
    RootDirective();
} RootDirective;

typedef struct UploadStoreDirective {
    bool is_set_;
    std::string value_;
    UploadStoreDirective();
} UploadStoreDirective;

struct CommonConfig {
    CommonConfig();
    RootDirective root_;
    AutoIndexDirective autoindex_;
    UploadStoreDirective upload_store_;
    ReturnDirective redirect_;
    off_t client_max_body_size_;
    std::map<int, ErrorPageDirective> error_page_;
    std::vector<std::string> index_files_;
    static const int INVALID_NUM = -1;
};

class LocationConfig {
   public:
    LocationConfig();
    // ---setter---
    void setPath(const std::string& p) { path_ = p; }
    void setRoot(const std::string& r) {
        common_config_.root_.value_ = r;
        common_config_.root_.is_set_ = true;
    }
    void addIndexFile(const std::string& file) {
        common_config_.index_files_.push_back(file);
    }
    void setAutoindex(bool on) {
        common_config_.autoindex_.value_ = on;
        common_config_.autoindex_.is_set_ = true;
    }
    void setClientMaxBodySize(off_t size) {
        common_config_.client_max_body_size_ = size;
    }
    void addErrorPage(int status, const ErrorPageDirective& ep) {
        common_config_.error_page_[status] = ep;
    }
    void setRedirect(const ReturnDirective& ret) {
        common_config_.redirect_ = ret;
    }
    void setUploadStore(const std::string& path) {
        common_config_.upload_store_.value_ = path;
        common_config_.upload_store_.is_set_ = true;
    }

    void addAllowedMethod(const Method& m) { allowed_methods_.push_back(m); }
    void setAllowedMethods(const std::vector<Method>& methods) {
        allowed_methods_ = methods;  // ベクター全体を代入（上書き）
    }

    void setCgiPath(const std::string& p) { cgi_path_ = p; }
    void setCgiExtension(const std::string& e) { cgi_extension_ = e; }
    const CommonConfig& getCommonConfig() const { return common_config_; }
    const std::vector<Method>& getAllowedMethods() const {
        return allowed_methods_;
    }
    const std::string& getCgiPath() const { return cgi_path_; }
    const std::string& getCgiExtension() const { return cgi_extension_; }
    const std::string& getPath() const { return path_; }

   private:
    CommonConfig common_config_;  // 共通設定
    // 許可するメソッドが入るだけ（例：GET HEAD POST DELETE）
    std::vector<Method> allowed_methods_;
    std::string cgi_path_;
    std::string cgi_extension_;
    std::string path_;
};

class ServerConfig {
   public:
    void addLocation(const LocationConfig& l) { locations_.push_back(l); }
    void setListen(const ListenDirective& l) { listens_ = l; }
    void addServerName(const std::string& name) {
        server_names_.push_back(name);
    }
    void setRoot(const std::string& r) {
        common_config_.root_.value_ = r;
        common_config_.root_.is_set_ = true;
    }
    void addIndexFile(const std::string& file) {
        common_config_.index_files_.push_back(file);
    }
    void setAutoindex(bool on) {
        common_config_.autoindex_.value_ = on;
        common_config_.autoindex_.is_set_ = true;
    }
    void setClientMaxBodySize(off_t size) {
        common_config_.client_max_body_size_ = size;
    }
    const ListenDirective& getListens() const { return listens_; }
    const std::vector<std::string>& getServerNames() const {
        return server_names_;
    }
    void addErrorPage(int status, const ErrorPageDirective& ep) {
        common_config_.error_page_[status] = ep;
    }
    void setRedirect(const ReturnDirective& ret) {
        common_config_.redirect_ = ret;
    }
    void setUploadStore(const std::string& path) {
        common_config_.upload_store_.value_ = path;
        common_config_.upload_store_.is_set_ = true;
    }

    // Parserから呼ぶために、common_config_ のゲッターが必要なら追加
    // (今回は内部で処理しているので不要、念のため)//testなどで使うかもしれない
    const CommonConfig& getCommonConfig() const { return common_config_; }
    const std::vector<LocationConfig>& getLocations() const {
        return locations_;
    }

   private:
    std::vector<LocationConfig> locations_;
    ListenDirective listens_;
    std::vector<std::string> server_names_;
    CommonConfig common_config_;  // 共通設定
};

class HttpConfig {
   public:
    // ---setter---
    // (パーサーが addServerConfig と呼んでいるため名前を合わせる)
    void addServerConfig(const ServerConfig& s) { servers_.push_back(s); }
    // parseCommonDirectiveテンプレートから呼ばれるため、
    // 共通設定用のセッター群をすべてここに追加する。
    void setRoot(const std::string& r) {
        common_config_.root_.value_ = r;
        common_config_.root_.is_set_ = true;
    }
    void addIndexFile(const std::string& file) {
        common_config_.index_files_.push_back(file);
    }
    void setAutoindex(bool on) {
        common_config_.autoindex_.value_ = on;
        common_config_.autoindex_.is_set_ = true;
    }
    void setClientMaxBodySize(off_t size) {
        common_config_.client_max_body_size_ = size;
    }
    void addErrorPage(int status, const ErrorPageDirective& ep) {
        common_config_.error_page_[status] = ep;
    }
    void setUploadStore(const std::string& path) {
        common_config_.upload_store_.value_ = path;
        common_config_.upload_store_.is_set_ = true;
    }
    // parserが使うため追加
    const CommonConfig& getCommonConfig() const { return common_config_; }
    const std::vector<ServerConfig>& getservers() const { return servers_; }

   private:
    std::vector<ServerConfig> servers_;
    CommonConfig common_config_;  // 共通設定
};
