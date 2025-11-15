#include <gtest/gtest.h>
#include "../src/http/HttpException.h"

TEST(HttpExceptionTest, MessageAndStatusCode) {
    HttpException ex(404, "Not Found");
    EXPECT_EQ(ex.status_code(), 404);
    EXPECT_STREQ(ex.what(), "Not Found");
}

TEST(HttpExceptionTest, DefaultMessageWhenEmpty) {
    HttpException ex(500);
    EXPECT_EQ(ex.status_code(), 500);
    EXPECT_STREQ(ex.what(), "HTTP Exception");
}

TEST(HttpExceptionTest, CopyConstructorCopiesFields) {
    HttpException original(400, "Bad Request");
    HttpException copy(original);
    EXPECT_EQ(copy.status_code(), 400);
    EXPECT_STREQ(copy.what(), "Bad Request");
}

TEST(HttpExceptionTest, AssignmentOperatorCopiesFields) {
    HttpException a(401, "Unauthorized");
    HttpException b(200, "OK");
    b = a;
    EXPECT_EQ(b.status_code(), 401);
    EXPECT_STREQ(b.what(), "Unauthorized");

    // self-assignment should be safe
    b = b;
    EXPECT_EQ(b.status_code(), 401);
    EXPECT_STREQ(b.what(), "Unauthorized");
}

TEST(HttpExceptionTest, PolymorphicCatchAsStdException) {
    try {
        throw HttpException(418, "I'm a teapot");
    } catch (const std::exception& e) {
        EXPECT_STREQ(e.what(), "I'm a teapot");
    }
}

TEST(HttpExceptionTest, StatusCode) {
    try {
        throw HttpException(500);
    } catch (const std::exception& e) {
        EXPECT_STREQ(e.what(), "HTTP Exception");
    }
}
