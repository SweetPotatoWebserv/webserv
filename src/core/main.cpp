#include "../config/HttpConfig.h"
#include "../event/Event.h"
#include "../http/Router.h"
#include "Server.h"

static HttpConfig createTestHttpConfig() {
    HttpConfig config;
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
        s1.common_config_.root_ = "docs/html/";
        s1.common_config_.autoindex_ = true;

        // location /
        {
            LocationConfig loc_root;
            loc_root.path_ = "/";
            loc_root.common_config_.root_ = "docs/html/";
            loc_root.allowed_methods_.push_back(MethodGET);
            loc_root.allowed_methods_.push_back(MethodHEAD);
            loc_root.common_config_.index_files_.push_back("index.html");
            s1.locations_.push_back(loc_root);
        }

        // location /img
        {
            LocationConfig loc_img;
            loc_img.path_ = "/img";
            loc_img.common_config_.root_ = "docs/img/";
            loc_img.allowed_methods_.push_back(MethodGET);
            loc_img.allowed_methods_.push_back(MethodHEAD);
            loc_img.common_config_.index_files_.push_back("icon.jpeg");
            s1.locations_.push_back(loc_img);
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
    return config;
}

int main(void) {
    HttpConfig config = createTestHttpConfig();
    Event ev;
    Router router(config);
    Server server(ev, router);
    server.start();
}
