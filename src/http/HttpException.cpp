#include "HttpException.h"

HttpException::HttpException(int status_code, const std::string& message)
    : status_code_(status_code), message_(message) {}

HttpException::HttpException(const HttpException& other) throw()
    : status_code_(other.status_code_), message_(other.message_) {}
HttpException& HttpException::operator=(const HttpException& other) throw() {
    if (this != &other) {
        this->status_code_ = other.status_code_;
        this->message_ = other.message_;
    }
    return *this;
}

HttpException::~HttpException() throw() {}
int HttpException::status_code() const { return status_code_; }
const char* HttpException::what() const throw() {
    if (!message_.empty()) {
        return message_.c_str();
    }
    static const std::string default_message = "HTTP Exception";
    return default_message.c_str();
}
