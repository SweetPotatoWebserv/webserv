#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define private public
#include "../config/HttpConfig.h"
#undef private
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include "../event/Event.h"
#include "../http/Router.h"
#include "Server.h"

// C++98 で使える構文のみでテスト用の HttpConfig を生成
static HttpConfig createTestHttpConfig() {
    HttpConfig config;

    // デフォルト設定
    config.defaults_.client_max_body_size_ = 5 * 1024 * 1024;  // 5MB // NOLINT
    config.defaults_.autoindex_ = false;
    config.defaults_.root_ = "www";
    config.defaults_.index_files_.push_back("index.html");
    config.defaults_.index_files_.push_back("index.htm");
    config.defaults_.error_page_.statuses.push_back(404);  // NOLINT
    config.defaults_.error_page_.target = "/404.html";
    config.defaults_.error_page_.override_status = -1;  // NOLINT

    // server 1: 127.0.0.1:8080 default_server
    {
        ServerConfig s1;
        s1.listens_.address = "127.0.0.1";
        s1.listens_.port = 8080;  // NOLINT
        s1.listens_.is_default_server = true;
        s1.server_names_.push_back("localhost");
        s1.server_names_.push_back("example.local");

        // server単位の上書き
        s1.common_config_.root_ = "www/site1";
        s1.common_config_.autoindex_ = true;

        // location /
        {
            LocationConfig loc_root;
            loc_root.path_ = "/";
            loc_root.allowed_methods_.push_back(MethodGET);
            loc_root.allowed_methods_.push_back(MethodHEAD);
            loc_root.common_config_.index_files_.push_back("index.html");
            s1.locations_.push_back(loc_root);
        }

        // location /upload
        {
            LocationConfig loc_upload;
            loc_upload.path_ = "/upload";
            loc_upload.allowed_methods_.push_back(MethodPOST);
            loc_upload.common_config_.upload_store_ = "uploads";
            s1.locations_.push_back(loc_upload);
        }

        // location /cgi-bin（CGI例）
        {
            LocationConfig loc_cgi;
            loc_cgi.path_ = "/cgi-bin";
            loc_cgi.allowed_methods_.push_back(MethodGET);
            loc_cgi.cgi_path_ = "/usr/bin/python3";  // 仮
            loc_cgi.cgi_extension_ = ".py";
            s1.locations_.push_back(loc_cgi);
        }

        config.servers_.push_back(s1);
    }

    // server 2: 127.0.0.1:8081（リダイレクト例）
    {
        ServerConfig s2;
        s2.listens_.address = "127.0.0.1";
        s2.listens_.port = 8081;  // NOLINT
        s2.listens_.is_default_server = false;
        s2.server_names_.push_back("redirect.local");

        // ルートへ来たら 301 /new-site へ
        s2.common_config_.redirect_.status = HttpStatus::MovedPermanently;
        s2.common_config_.redirect_.target = "/new-site";

        s2.common_config_.index_files_.clear();
        s2.common_config_.index_files_.push_back("home.html");

        // location /new-site
        {
            LocationConfig loc_new;
            loc_new.path_ = "/new-site";
            loc_new.allowed_methods_.push_back(MethodGET);
            loc_new.allowed_methods_.push_back(MethodHEAD);
            s2.locations_.push_back(loc_new);
        }

        config.servers_.push_back(s2);
    }

    return config;
}

int main(void) {
    HttpConfig config = createTestHttpConfig();
    Event ev;
    Router router(config);
    Server server(ev, router);
    server.start();
}
