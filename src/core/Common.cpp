#include "Common.h"

#include <sys/socket.h>

// 定数
const char* const HTTP_VERSION = "HTTP/1.1";
const uint16_t DEFAULT_PORT = 80;
const int SOCKET_DOMAIN = AF_INET;
const int SOCKET_TYPE = SOCK_STREAM;
const int SOCKET_PROTOCOL = 0;
const int SOCKET_BACKLOG = 128;

std::map<int, std::string> HttpStatus::createReasonMap() {
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

std::string HttpStatus::reason(int code) {
    static const std::map<int, std::string> ReasonMap = createReasonMap();
    std::map<int, std::string>::const_iterator it = ReasonMap.find(code);
    if (it != ReasonMap.end()) return it->second;
    return "Unknown Status";
}

bool HostHeader::resolve_ipv4(const std::string& host, uint16_t port,
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

    struct sockaddr_in* addr_in =
        reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    std::memcpy(&out_addr, addr_in, sizeof(sockaddr_in));
    out_addr.sin_port = htons(port);

    freeaddrinfo(res);
    return true;
}

Socket::Socket() {
    fd_ = ::socket(SOCKET_DOMAIN, SOCKET_TYPE, SOCKET_PROTOCOL);
    if (fd_ < 0) {
        throw std::runtime_error("socket failed: " +
                                 std::string(strerror(errno)));
    }
}

Socket::~Socket() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

Socket Socket::listen_tcp(const std::string& host, uint16_t port) {
    Socket server_fd;
    sockaddr_in addr = {};
    if (HostHeader::resolve_ipv4(host, port, addr)) {
        std::cerr << "resolve_ipv4 failed";
    }
    if (::bind(server_fd.fd_, reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) == -1) {
        throw std::runtime_error("bind failed: " +
                                 std::string(strerror(errno)));
    }
    if (::listen(server_fd.fd_, SOCKET_BACKLOG) == -1) {
        throw std::runtime_error("listen failed: " +
                                 std::string(strerror(errno)));
    }

    return server_fd;
}

int Socket::getFd() const { return fd_; }
