#pragma once

#include <sys/types.h>

#include <exception>
#include <string>

enum class StatusCode : u_int16_t {
    kOk = 200,
    kRedirect = 300,
    kClientError = 400,
    kServerError = 500
};

class CgiExecutionException : public std::exception {
   public:
    CgiExecutionException(const std::string &message, StatusCode statusCode)
        : message_(message), statusCode_(statusCode) {}
    virtual ~CgiExecutionException() throw() {}
    virtual const char *what() const throw() { return message_.c_str(); }
    StatusCode getStatusCode() const { return statusCode_; }

   private:
    std::string message_;
    StatusCode statusCode_;
};

class CgiExecutor {
   public:
    CgiExecutor();
    ~CgiExecutor();

    std::string execute(const std::string &scriptPath, char *const argv[],
                        char *const envp[], const std::string &requestBody);

   private:
    pid_t pid_;
    int pipeIn_[2];
    int pipeOut_[2];

    void executeChildProcess(const std::string &scriptPath, char *const argv[],
                             char *const envp[]);
    std::string readParentProcess(const std::string &requestBody);
    static std::string getScriptDirectory(const std::string &scriptPath);
};
