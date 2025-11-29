#include "Common.h"

#include <fcntl.h>
#include <sys/socket.h>

#include <cstring>
#include <stdexcept>

#include "Socket.h"

// 定数
const char* const HTTP_VERSION = "HTTP/1.1";
const uint16_t DEFAULT_PORT = 8080;
const char* const DEFAULT_ADDRESS = "0.0.0.0";
const char* const HTTP_LINE_END = "\r\n";
const char* const COLON = ":";
const std::vector<std::string>::size_type HEADER_FIELD_NUM = 2;
const char* const CONTENT_LENGTH = "content-length";
const char* const TRANSFER_ENCODING = "transfer-encoding";
const char* const CONTENT_TYPE = "content-type";
const char* const HOST = "host";
const char* const CHUNKED = "chunked";
const char* const HTTP_HEADER_END = "\r\n\r\n";
const int HTTP_LINE_END_LEN = 2;
const int HTTP_HEADER_END_LEN = 4;
const char* const QUESTION_MARK = "?";
const int DECIMAL = 10;
const uint16_t MIN_PORT_NUMBER = 0;
const uint16_t MAX_PORT_NUMBER = 65535;
const int MIN_ERROR_STATUS_CODE = 300;
const int MAX_ERROR_STATUS_CODE = 599;
const int MIN_VALID_STATUS_CODE = 200;
const int MAX_VALID_STATUS_CODE = 501;
const int MIN_REDIRECT_STATUS_CODE = 301;
const int MAX_REDIRECT_STATUS_CODE = 308;

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
    hints.ai_family = fd::Socket::SOCKET_DOMAIN;
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

const std::string& HostHeader::getAddress() const { return address_; }
uint16_t HostHeader::getPort() const { return port_; }

Method string_to_method(const std::string& method_str) {
    if (method_str == "GET") return MethodGET;
    if (method_str == "HEAD") return MethodHEAD;
    if (method_str == "POST") return MethodPOST;
    if (method_str == "DELETE") return MethodDELETE;
    return MethodUNKNOWN;
}
