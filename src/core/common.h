#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

enum Method : std::uint8_t {
    MethodGET,
    MethodHEAD,
    MethodPOST,
    MethodDELETE,
};

// uint16_t が c++98 だと標準でサポートされるか怪しいため、typedef で定義
typedef unsigned short uint16_t;

typedef long long off_t;

extern const char* const HTTP_VERSION;
extern const uint16_t DEFAULT_PORT;

class HttpStatus {
   public:
    enum Code {  // NOLINT
        OK = 200,
        Created = 201,
        NoContent = 204,

        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,

        BadRequest = 400,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        RequestTimeout = 408,
        PayloadTooLarge = 413,
        URITooLong = 414,

        InternalServerError = 500,
        NotImplemented = 501
    };

    static std::string reason(int code);

   private:
    static std::map<int, std::string> createReasonMap();
};
