#include <gtest/gtest.h>
#include "../src/http/MimeTypes.h"

TEST(MimeTypes, KnownHtml) {
    EXPECT_EQ(MimeTypes::get_mime_type("index.html"), "text/html");
    EXPECT_EQ(MimeTypes::get_mime_type("index.HTML"), "text/html");
    EXPECT_EQ(MimeTypes::get_mime_type("main.htm"), "text/html");
}

TEST(MimeTypes, StylesAndScripts) {
    EXPECT_EQ(MimeTypes::get_mime_type("style.css"), "text/css");
    EXPECT_EQ(MimeTypes::get_mime_type("app.js"), "application/javascript");
    EXPECT_EQ(MimeTypes::get_mime_type("data.json"), "application/json");
}

TEST(MimeTypes, ImagesCaseInsensitive) {
    EXPECT_EQ(MimeTypes::get_mime_type("photo.jpg"), "image/jpeg");
    EXPECT_EQ(MimeTypes::get_mime_type("photo.JPEG"), "image/jpeg");
    EXPECT_EQ(MimeTypes::get_mime_type("icon.PNG"), "image/png");
    EXPECT_EQ(MimeTypes::get_mime_type("anim.Gif"), "image/gif");
}

TEST(MimeTypes, ArchivesAndCompressed) {
    EXPECT_EQ(MimeTypes::get_mime_type("archive.zip"), "application/zip");
    EXPECT_EQ(MimeTypes::get_mime_type("backup.tar"), "application/x-tar");
    EXPECT_EQ(MimeTypes::get_mime_type("archive.tar.gz"), "application/gzip");
}

TEST(MimeTypes, CodeAndCgi) {
    EXPECT_EQ(MimeTypes::get_mime_type("index.php"), "application/x-httpd-php");
    EXPECT_EQ(MimeTypes::get_mime_type("script.py"), "text/x-python");
    EXPECT_EQ(MimeTypes::get_mime_type("lib.c"), "text/x-c");
    EXPECT_EQ(MimeTypes::get_mime_type("lib.CPP"), "text/x-c++");
}

TEST(MimeTypes, UnknownOrNoExtension) {
    EXPECT_EQ(MimeTypes::get_mime_type("README"), "application/octet-stream");
    EXPECT_EQ(MimeTypes::get_mime_type("file.unknownext"), "application/octet-stream");
    EXPECT_EQ(MimeTypes::get_mime_type(".bashrc"), "application/octet-stream");
    EXPECT_EQ(MimeTypes::get_mime_type("trailingdot."), "application/octet-stream");
}

