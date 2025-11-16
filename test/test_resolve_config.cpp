#include <gtest/gtest.h>

#include "../src/http/ResolveConfig.h"
#include "../src/config/HttpConfig.h"
#include "../src/core/Common.h"

// Helpers to construct directives
static CommonConfig make_http_common(
    off_t body = CommonConfig::INVALID_NUM,
    const char* root = NULL,
    bool autoindex_set = false,
    bool autoindex_value = false,
    const char* upload_store = NULL,
    int redirect_status = CommonConfig::INVALID_NUM,
    const char* redirect_target = NULL,
    const std::vector<std::pair<int, std::pair<std::string, int> > >& error_pages = std::vector<std::pair<int, std::pair<std::string, int> > >(),
    const std::vector<std::string>& index_files = std::vector<std::string>()) {
    CommonConfig cc;
    if (body != CommonConfig::INVALID_NUM) cc.client_max_body_size_ = body;
    if (root) {
        cc.root_.value_ = root;
        cc.root_.is_set_ = true;
    }
    if (autoindex_set) {
        cc.autoindex_.is_set_ = true;
        cc.autoindex_.value_ = autoindex_value;
    }
    if (upload_store) {
        cc.upload_store_.is_set_ = true;
        cc.upload_store_.value_ = upload_store;
    }
    if (redirect_status != CommonConfig::INVALID_NUM) {
        cc.redirect_.status = redirect_status;
        if (redirect_target) cc.redirect_.target = redirect_target;
    }
    for (std::vector<std::pair<int, std::pair<std::string, int> > >::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
        int code = it->first;
        std::string tgt = it->second.first;
        int override = it->second.second;
        ErrorPageDirective ep(tgt, override);
        cc.error_page_[code] = ep;
    }
    for (std::vector<std::string>::const_iterator it = index_files.begin(); it != index_files.end(); ++it) {
        cc.index_files_.push_back(*it);
    }
    return cc;
}

// --- Tests ---

TEST(ResolveConfigTest, DefaultResolution_NoValuesSet) {
    HttpConfig http;  // default
    ServerConfig server;  // default (listen is default 127.0.0.1:8080)
    LocationConfig location;  // default

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    EXPECT_EQ(r.client_max_body_size_, CommonConfig::INVALID_NUM);
    EXPECT_EQ(r.redirect_.status, CommonConfig::INVALID_NUM);
    EXPECT_FALSE(r.root_.is_set_);
    EXPECT_FALSE(r.upload_store_.is_set_);
    EXPECT_FALSE(r.autoindex_.is_set_);
    EXPECT_TRUE(r.error_page_.empty());
    EXPECT_TRUE(r.index_files_.empty());
    EXPECT_TRUE(r.allowed_methods_.empty());
    EXPECT_TRUE(r.cgi_path_.empty());
    EXPECT_TRUE(r.cgi_extension_.empty());
    EXPECT_TRUE(r.path_.empty());

    // From server default listen
    EXPECT_EQ(r.listens_.address, std::string(DEFAULT_ADDRESS));
    EXPECT_EQ(r.listens_.port, DEFAULT_PORT);
    EXPECT_FALSE(r.listens_.is_default_server);
    EXPECT_TRUE(r.server_names_.empty());
}

TEST(ResolveConfigTest, HttpLevelOnly_PopulatesFields) {
    // http-level config set
    CommonConfig cc = make_http_common(
        12345, "/var/www/http", true, true, "/var/upload",
        301, "/moved",
        std::vector<std::pair<int, std::pair<std::string, int> > >{
            std::make_pair(404, std::make_pair(std::string("/404.html"), 200)),
            std::make_pair(500, std::make_pair(std::string("/50x.html"), CommonConfig::INVALID_NUM))},
        std::vector<std::string>{"index.html", "index.htm"}
    );

    HttpConfig http;
    http.setDefaults(cc);
    ServerConfig server;  // default
    LocationConfig location;  // default

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    EXPECT_EQ(r.client_max_body_size_, 12345);
    ASSERT_TRUE(r.root_.is_set_);
    EXPECT_EQ(r.root_.value_, std::string("/var/www/http"));
    ASSERT_TRUE(r.autoindex_.is_set_);
    EXPECT_TRUE(r.autoindex_.value_);
    ASSERT_TRUE(r.upload_store_.is_set_);
    EXPECT_EQ(r.upload_store_.value_, std::string("/var/upload"));
    EXPECT_EQ(r.redirect_.status, 301);
    EXPECT_EQ(r.redirect_.target, std::string("/moved"));
    ASSERT_EQ(r.index_files_.size(), static_cast<size_t>(2));
    EXPECT_EQ(r.index_files_[0], std::string("index.html"));
    EXPECT_EQ(r.index_files_[1], std::string("index.htm"));
    ASSERT_EQ(r.error_page_.size(), static_cast<size_t>(2));
    EXPECT_EQ(r.error_page_[404].target, std::string("/404.html"));
    EXPECT_EQ(r.error_page_[404].override_status, 200);
    EXPECT_EQ(r.error_page_[500].target, std::string("/50x.html"));
    EXPECT_EQ(r.error_page_[500].override_status, CommonConfig::INVALID_NUM);

    // Server-derived fields
    EXPECT_EQ(r.listens_.address, std::string(DEFAULT_ADDRESS));
    EXPECT_TRUE(r.server_names_.empty());
    EXPECT_TRUE(r.allowed_methods_.empty());
}

TEST(ResolveConfigTest, ServerOverridesHttp_AndProvidesServerFields) {
    // http-level base
    HttpConfig http;
    http.setDefaults(make_http_common(100, "/h-root", true, true, NULL,
                                      CommonConfig::INVALID_NUM, NULL));

    // server overrides common and sets listen + names
    ServerConfig server;
    server.setRoot("/s-root");
    server.setAutoindex(false);
    server.setClientMaxBodySize(200);
    server.addIndexFile("home.html");

    ListenDirective ld;
    ld.address = "0.0.0.0";
    ld.port = 80;
    ld.is_default_server = true;
    server.setListen(ld);
    server.addServerName("example.com");
    server.addServerName("www.example.com");

    LocationConfig location;  // no overrides here

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    // server overrides http
    EXPECT_EQ(r.client_max_body_size_, 200);
    ASSERT_TRUE(r.root_.is_set_);
    EXPECT_EQ(r.root_.value_, std::string("/s-root"));
    ASSERT_TRUE(r.autoindex_.is_set_);
    EXPECT_FALSE(r.autoindex_.value_);
    ASSERT_EQ(r.index_files_.size(), static_cast<size_t>(1));
    EXPECT_EQ(r.index_files_[0], std::string("home.html"));

    // server-provided only fields
    EXPECT_EQ(r.listens_.address, std::string("0.0.0.0"));
    EXPECT_EQ(r.listens_.port, static_cast<uint16_t>(80));
    EXPECT_TRUE(r.listens_.is_default_server);
    ASSERT_EQ(r.server_names_.size(), static_cast<size_t>(2));
    EXPECT_EQ(r.server_names_[0], std::string("example.com"));
    EXPECT_EQ(r.server_names_[1], std::string("www.example.com"));
}

TEST(ResolveConfigTest, LocationOverridesAndAddsLocationOnlyFields) {
    // Base http and server
    HttpConfig http;
    http.setDefaults(make_http_common(100, "/h", true, false, NULL, 302, "/r"));

    ServerConfig server;
    server.setRoot("/s");
    server.setAutoindex(true);
    server.setClientMaxBodySize(200);
    server.addIndexFile("server.html");
    ListenDirective ld;
    ld.address = "127.0.0.1";
    ld.port = 8081;
    ld.is_default_server = false;
    server.setListen(ld);

    // Location overrides and adds location-specifics
    LocationConfig location;
    location.setPath("/app");
    location.setRoot("/l");
    location.setAutoindex(false);
    location.setClientMaxBodySize(300);
    location.addIndexFile("loc.html");
    location.addAllowedMethod(MethodGET);
    location.addAllowedMethod(MethodPOST);
    location.setCgiPath("/usr/bin/php-cgi");
    location.setCgiExtension(".php");

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    // Location overrides common
    EXPECT_EQ(r.client_max_body_size_, 300);
    ASSERT_TRUE(r.root_.is_set_);
    EXPECT_EQ(r.root_.value_, std::string("/l"));
    ASSERT_TRUE(r.autoindex_.is_set_);
    EXPECT_FALSE(r.autoindex_.value_);
    ASSERT_EQ(r.index_files_.size(), static_cast<size_t>(1));
    EXPECT_EQ(r.index_files_[0], std::string("loc.html"));

    // Location-only fields present
    ASSERT_EQ(r.allowed_methods_.size(), static_cast<size_t>(2));
    EXPECT_EQ(r.allowed_methods_[0], MethodGET);
    EXPECT_EQ(r.allowed_methods_[1], MethodPOST);
    EXPECT_EQ(r.cgi_path_, std::string("/usr/bin/php-cgi"));
    EXPECT_EQ(r.cgi_extension_, std::string(".php"));
    EXPECT_EQ(r.path_, std::string("/app"));

    // Server-only fields remain from server
    EXPECT_EQ(r.listens_.address, std::string("127.0.0.1"));
    EXPECT_EQ(r.listens_.port, static_cast<uint16_t>(8081));

    // Http-only fields not overridden by server/location (due to API)
    EXPECT_EQ(r.redirect_.status, 302);
    EXPECT_EQ(r.redirect_.target, std::string("/r"));
}

TEST(ResolveConfigTest, HttpOnlyDirectivesRemainWhenNoOverrides) {
    // Set http-only directives: upload_store, error_page, redirect
    std::vector<std::pair<int, std::pair<std::string, int> > > eps;
    eps.push_back(std::make_pair(404, std::make_pair(std::string("/custom404.html"), 200)));
    eps.push_back(std::make_pair(403, std::make_pair(std::string("/403.html"), CommonConfig::INVALID_NUM)));

    HttpConfig http;
    http.setDefaults(make_http_common(CommonConfig::INVALID_NUM, NULL, false, false,
                                      "/uploads",
                                      301, "/moved", eps));

    ServerConfig server;  // no overrides available for these via API
    LocationConfig location;  // same

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    ASSERT_TRUE(r.upload_store_.is_set_);
    EXPECT_EQ(r.upload_store_.value_, std::string("/uploads"));
    EXPECT_EQ(r.redirect_.status, 301);
    EXPECT_EQ(r.redirect_.target, std::string("/moved"));
    ASSERT_EQ(r.error_page_.size(), static_cast<size_t>(2));
    EXPECT_EQ(r.error_page_[404].target, std::string("/custom404.html"));
    EXPECT_EQ(r.error_page_[404].override_status, 200);
    EXPECT_EQ(r.error_page_[403].target, std::string("/403.html"));
    EXPECT_EQ(r.error_page_[403].override_status, CommonConfig::INVALID_NUM);
}

