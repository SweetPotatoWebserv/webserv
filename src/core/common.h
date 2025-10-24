#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>


enum Method {
    GET,
    HEAD,
    POST,
    DELETE,
};

// uint16_t が c++98 だと標準でサポートされるか怪しいため、typedef で定義
typedef unsigned short uint16_t;

typedef long long off_t;

extern const char* HTTP_VERSION;
extern const uint16_t DEFAULT_PORT;



// TODO 実装を .cpp に移す
class HttpStatus {
public:
    enum Code {
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

    static std::string reason(int code) {
        static const std::map<int, std::string> reasons = createReasonMap();
        std::map<int, std::string>::const_iterator it = reasons.find(code);
        if (it != reasons.end())
            return it->second;
        return "Unknown Status";
    }

private:
    static std::map<int, std::string> createReasonMap() {
        std::map<int, std::string> m;

        // 2xx
        m[OK] = "OK";
        m[Created] = "Created";
        m[NoContent] = "No Content";

        // 3xx
        m[MovedPermanently] = "Moved Permanently";
        m[Found] = "Found";
        m[SeeOther] = "See Other";

        // 4xx
        m[BadRequest] = "Bad Request";
        m[Forbidden] = "Forbidden";
        m[NotFound] = "Not Found";
        m[MethodNotAllowed] = "Method Not Allowed";
        m[RequestTimeout] = "Request Timeout";
        m[PayloadTooLarge] = "Payload Too Large";
        m[URITooLong] = "URI Too Long";

        // 5xx
        m[InternalServerError] = "Internal Server Error";
        m[NotImplemented] = "Not Implemented";

        return m;
    }
};

