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
int HttpException::status_code() const { return status_code_; }
const char* HttpException::what() const throw() { return message_.c_str(); }
