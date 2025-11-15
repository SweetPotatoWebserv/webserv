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
    ErrorPageDirective(std::string& t, int o);
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

    uint16_t port;
    bool is_default_server;
    ListenDirective();
} ListenDirective;

class CommonConfig {
   public:
    CommonConfig();

    // ---setter---
    void setRoot(const std::string& path) {
        root_ = path;
        root_is_set_ = true;  // root が設定されたことを記録
    }
    void setAutoindex(bool on) {
        autoindex_ = on;
        autoindex_is_set_ = true;
    }
    void setUploadStore(const std::string& store_path_) {
        upload_store_ = store_path_;
        upload_store_is_set_ = true;
    }
    void setRedirect(const ReturnDirective& ret) {
        redirect_ = ret;
        redirect_is_set_ = true;
    }
    void addErrorPage(int status, const ErrorPageDirective& ep) {
        error_page_[status] = ep;
    }
    void setClientMaxBodySize(off_t size) { client_max_body_size_ = size; }
    void addIndexFile(const std::string& file) { index_files_.push_back(file); }

    bool isRootSet() const { return root_is_set_; }
    const std::string& getRoot() const { return root_; }

    bool isAutoindexSet() const { return autoindex_is_set_; }
    bool getAutoindex() const { return autoindex_; }

    bool isUploadStoreSet() const { return upload_store_is_set_; }
    const std::string& getUploadStore() const { return upload_store_; }

    bool isRedirectSet() const { return redirect_is_set_; }
    const ReturnDirective& getRedirect() const { return redirect_; }

    off_t getClientMaxBodySize() const { return client_max_body_size_; }
    const std::map<int, ErrorPageDirective>& getErrorPages() const {
        return error_page_;
    }
    const std::vector<std::string>& getIndexFiles() const {
        return index_files_;
    }

   private:
    bool root_is_set_;
    std::string root_;

    bool autoindex_is_set_;
    bool autoindex_;

    bool upload_store_is_set_;  // 自動的にis_setを設定するためprivate管理
    std::string upload_store_;

    ReturnDirective redirect_;
    bool redirect_is_set_;

    off_t client_max_body_size_;
    std::map<int, ErrorPageDirective> error_page_;
    std::vector<std::string> index_files_;
};

class LocationConfig {
   public:
    // ---setter---
    void setPath(const std::string& p) { path_ = p; }
    void setRoot(const std::string& r) { common_config_.setRoot(r); }
    void addIndexFile(const std::string& file) {
        common_config_.addIndexFile(file);
    }
    void setAutoindex(bool on) { common_config_.setAutoindex(on); }
    void setClientMaxBodySize(off_t size) {
        common_config_.setClientMaxBodySize(size);
    }

    void addAllowedMethod(const Method& m) { allowed_methods_.push_back(m); }
    void setCgiPath(const std::string& p) { cgi_path_ = p; }
    void setCgiExtension(const std::string& e) { cgi_extension_ = e; }

    // 継承設定（locationの場合、server で設定されたものを継承する関数）
    void resolveDefaults(const CommonConfig& server_common) {  //<-親の設定
        // root
        if (!common_config_.isRootSet()) {
            // 自分が設定していないなら、親の設定をコピー
            if (server_common.isRootSet()) {
                common_config_.setRoot(server_common.getRoot());
            }
        }
        // autoindex
        if (!common_config_.isAutoindexSet()) {
            if (server_common.isAutoindexSet()) {
                common_config_.setAutoindex(server_common.getAutoindex());
            }
        }
        // client_max_body_size
        if (common_config_.getClientMaxBodySize() == -1) {
            common_config_.setClientMaxBodySize(
                server_common.getClientMaxBodySize());
        }
        // upload_store
        if (!common_config_.isUploadStoreSet()) {
            if (server_common.isUploadStoreSet()) {
                common_config_.setUploadStore(server_common.getUploadStore());
            }
        }
        // index_files (vectorが空なら継承)
        if (common_config_.getIndexFiles().empty()) {
            const std::vector<std::string>& parent_index =
                server_common.getIndexFiles();
            for (size_t i = 0; i < parent_index.size(); ++i) {
                common_config_.addIndexFile(parent_index[i]);
            }
        }
        // error_pages (mapが空なら継承)
        //  ※Nginx仕様:
        //  自分の階層で1つでも定義したら継承しない。空の時だけ継承。
        if (common_config_.getErrorPages().empty()) {
            std::map<int, ErrorPageDirective>::const_iterator it;
            for (it = server_common.getErrorPages().begin();
                 it != server_common.getErrorPages().end(); ++it) {
                common_config_.addErrorPage(it->first, it->second);
            }
        }
        // return (redirect)
        //  ※Nginx仕様: 自分が設定していなければ親を使う
        if (!common_config_.isRedirectSet()) {
            if (server_common.isRedirectSet()) {
                common_config_.setRedirect(server_common.getRedirect());
            }
        }
    }

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

    void setRoot(const std::string& r) { common_config_.setRoot(r); }
    void addIndexFile(const std::string& file) {
        common_config_.addIndexFile(file);
    }
    void setAutoindex(bool on) { common_config_.setAutoindex(on); }
    void setClientMaxBodySize(off_t size) {
        common_config_.setClientMaxBodySize(size);
    }

    void resolveDefaults(const CommonConfig& http_common) {  //<-親の設定
        //ロジックは Locationと同じ

        if (!common_config_.isRootSet() && http_common.isRootSet()) {
            common_config_.setRoot(http_common.getRoot());
        }
        if (!common_config_.isAutoindexSet() && http_common.isAutoindexSet()) {
            common_config_.setAutoindex(http_common.getAutoindex());
        }
        if (common_config_.getClientMaxBodySize() == -1) {
            common_config_.setClientMaxBodySize(
                http_common.getClientMaxBodySize());
        }
        if (!common_config_.isUploadStoreSet() &&
            http_common.isUploadStoreSet()) {
            common_config_.setUploadStore(http_common.getUploadStore());
        }

        if (common_config_.getIndexFiles().empty()) {
            const std::vector<std::string>& parent_index =
                http_common.getIndexFiles();
            for (size_t i = 0; i < parent_index.size(); ++i) {
                common_config_.addIndexFile(parent_index[i]);
            }
        }

        if (common_config_.getErrorPages().empty()) {
            std::map<int, ErrorPageDirective>::const_iterator it;
            for (it = http_common.getErrorPages().begin();
                 it != http_common.getErrorPages().end(); ++it) {
                common_config_.addErrorPage(it->first, it->second);
            }
        }

        // return は http
        //今回は書かない。Nginxの設定思想に準じていないため

        // 自分がhttpの設定を含んだ設定になったので、子であるlocation
        // たちに設定を配る
        for (size_t i = 0; i < locations_.size(); ++i) {
            locations_[i].resolveDefaults(this->common_config_);
        }
    }

    // Parserから呼ぶために、common_config_ のゲッターが必要なら追加
    // (今回は内部で処理しているので不要、念のため)//testなどで使うかもしれない
    const CommonConfig& getCommonConfig() const { return common_config_; }

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
    void setDefaults(const CommonConfig& c) { common_config_ = c; }
    // parserが使うため追加
    const CommonConfig& getCommonConfig() const { return common_config_; }
    std::vector<ServerConfig>& getservers() {
        return servers_;
    }  // non-const参照 (中身を書き換えるため)

   private:
    std::vector<ServerConfig> servers_;
    CommonConfig common_config_;  // 共通設定
};