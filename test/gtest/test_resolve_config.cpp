#include <gtest/gtest.h>

#include "../src/http/ResolveConfig.h"
#include "../src/config/HttpConfig.h"
#include "../src/core/Common.h"

// --- Tests ---

TEST(ResolveConfigTest, DefaultResolution_NoValuesSet) {
    HttpConfig http;
    ServerConfig server;
    LocationConfig location;

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    EXPECT_EQ(r.client_max_body_size_, -1);
    EXPECT_EQ(r.redirect_.status, -1);
    EXPECT_FALSE(r.root_.is_set_);
    EXPECT_FALSE(r.upload_store_.is_set_);
    EXPECT_FALSE(r.autoindex_.is_set_);
    EXPECT_TRUE(r.index_files_.empty());
    EXPECT_FALSE(r.allowed_methods_.empty());
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
    // http-level config set via public API
    HttpConfig http;
    http.setClientMaxBodySize(12345);
    http.setRoot("/var/www/http");
    http.setAutoindex(true);
    http.setUploadStore("/var/upload");
    http.addIndexFile("index.html");
    http.addIndexFile("index.htm");
    {
        // HttpConfig has no setRedirect API; inject for test
        CommonConfig& hcc = const_cast<CommonConfig&>(http.getCommonConfig());
        hcc.redirect_.status = 301;
        hcc.redirect_.target = "/moved";
    }
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

    // Server-derived fields
    EXPECT_EQ(r.listens_.address, std::string(DEFAULT_ADDRESS));
    EXPECT_TRUE(r.server_names_.empty());
    EXPECT_FALSE(r.allowed_methods_.empty());
}

TEST(ResolveConfigTest, ServerOverridesHttp_AndProvidesServerFields) {
    // http-level base
    HttpConfig http;
    http.setClientMaxBodySize(100);
    http.setRoot("/h-root");
    http.setAutoindex(true);

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
    http.setClientMaxBodySize(100);
    http.setRoot("/h");
    http.setAutoindex(false);
    {
        CommonConfig& hcc = const_cast<CommonConfig&>(http.getCommonConfig());
        hcc.redirect_.status = 302;
        hcc.redirect_.target = "/r";
    }

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
    std::vector<Method> methods;
    methods.push_back(MethodGET);
    methods.push_back(MethodPOST);
    location.setAllowedMethods(methods);
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
    HttpConfig http;
    http.setUploadStore("/uploads");
    {
        CommonConfig& hcc = const_cast<CommonConfig&>(http.getCommonConfig());
        hcc.redirect_.status = 301;
        hcc.redirect_.target = "/moved";
    }

    ServerConfig server;  // no overrides available for these via API
    LocationConfig location;  // same

    ResolveConfig r = ResolveConfig::resolve_config(http, server, location);

    ASSERT_TRUE(r.upload_store_.is_set_);
    EXPECT_EQ(r.upload_store_.value_, std::string("/uploads"));
    EXPECT_EQ(r.redirect_.status, 301);
    EXPECT_EQ(r.redirect_.target, std::string("/moved"));
}
