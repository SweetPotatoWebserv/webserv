#pragma once

#include <netdb.h>

#include <exception>
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

extern const int SOCKET_DOMAIN;
extern const int SOCKET_TYPE;
extern const int SOCKET_PROTOCOL;
extern const int SOCKET_BACKLOG;

class HostHeader {
   public:
    static bool resolve_ipv4(const std::string& host, uint16_t port,
                             struct sockaddr_in& out_addr) {
        struct addrinfo hints;
        struct addrinfo* res = NULL;

        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = SOCKET_DOMAIN;
        hints.ai_socktype = SOCK_STREAM;

        int ret = getaddrinfo(host.c_str(), NULL, &hints, &res);
        if (ret != 0 || res == NULL) {
            return false;
        }

        struct sockaddr_in* addr_in = (struct sockaddr_in*)res->ai_addr;
        std::memcpy(&out_addr, addr_in, sizeof(sockaddr_in));
        out_addr.sin_port = htons(port);

        freeaddrinfo(res);
        return true;
    }

    HostHeader() : port_(DEFAULT_PORT) {}

   private:
    std::string address_;
    uint16_t port_;
};
