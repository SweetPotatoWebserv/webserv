#pragma once

#include <stdexcept>
#include <string>

class HttpException : public std::exception {
   public:
    explicit HttpException(int status_code, const std::string& message = "");
    HttpException(const HttpException& other) throw();
    HttpException& operator=(const HttpException& other) throw();
    int status_code() const;
    virtual const char* what() const throw();
    virtual ~HttpException() throw();

   private:
    int status_code_;
    std::string message_;
};
