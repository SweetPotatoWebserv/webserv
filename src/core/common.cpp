#include "common.h"

const char* const HTTP_VERSION = "HTTP/1.1";
const uint16_t DEFAULT_PORT = 80;

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
