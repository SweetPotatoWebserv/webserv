#pragma once

#include <sys/types.h>

#include <exception>
#include <string>

#include "../core/Common.h"

class CgiExecutionException : public std::exception {
   public:
    CgiExecutionException(const std::string &message,
                          HttpStatus::Code statusCode)
        : message_(message), statusCode_(statusCode) {}
    CgiExecutionException(const CgiExecutionException &other) throw()
        : std::exception(other),
          message_(other.message_),
          statusCode_(other.statusCode_) {}
    virtual ~CgiExecutionException() throw() {}

    virtual const char *what() const throw() { return message_.c_str(); }
    HttpStatus::Code getStatusCode() const { return statusCode_; }

   private:
    std::string message_;
    HttpStatus::Code statusCode_;
};

class CgiExecutor {
   public:
    CgiExecutor();
    ~CgiExecutor();

    std::string execute(const std::string &scriptPath, char *const argv[],
                        char *const envp[], const std::string &requestBody);
    static void safeClose(int &fd);

   private:
    pid_t pid_;
    int pipeIn_[2];
    int pipeOut_[2];

    void executeChildProcess(const std::string &scriptPath, char *const argv[],
                             char *const envp[]);
    std::string readParentProcess(const std::string &requestBody);

    static void writeAll(int fd, const char *buffer, size_t size);
    static void checkChildExitStatus(int status);
    static std::string getScriptDirectory(const std::string &scriptPath);
    static std::string getScriptBasename(const std::string &scriptPath);
};
