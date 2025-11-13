#pragma once

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <vector>

enum Method { MethodGET, MethodHEAD, MethodPOST, MethodDELETE };

// uint16_t が c++98 だと標準でサポートされるか怪しいため、typedef で定義
typedef unsigned short uint16_t;

extern const char* const HTTP_VERSION;
extern const uint16_t DEFAULT_PORT;
extern const char* const DEFAULT_ADDRESS;
extern const int SOCKET_DOMAIN;
extern const int SOCKET_TYPE;
extern const int SOCKET_PROTOCOL;
extern const int SOCKET_BACKLOG;
extern const char* const HTTP_LINE_END;
extern const char* const COLON;
extern const int HEADER_FIELD_NUM;
extern const char* const CONTENT_LENGTH;
extern const char* const TRANSFER_ENCODING;

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

class HostHeader {
   public:
    static bool resolve_ipv4(const std::string& host, uint16_t port,
                             struct sockaddr_in& out_addr);
    HostHeader() : address_(DEFAULT_ADDRESS), port_(DEFAULT_PORT) {}
    HostHeader(const std::string& address, uint16_t port)
        : address_(address), port_(port) {}
    const std::string& getAddress() const;
    uint16_t getPort() const;

   private:
    std::string address_;
    uint16_t port_;
};

class Socket {
   public:
    static Socket listen_tcp(const std::string& host, uint16_t port);
    int getFd() const;
    Socket& operator=(const Socket& rhs);
    Socket(const Socket&);
    ~Socket();
    static void set_nonblocking(int fd);

   private:
    int fd_;
    Socket();
};
