#pragma once

#include <sys/types.h>

#include <exception>
#include <string>

#include "../core/Common.h"

struct CgiResult {
    pid_t pid;
    int readFd;   // 親が読む (CGIのstdout)
    int writeFd;  // 親が書く (CGIのstdin)
};

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

    CgiResult execute(const std::string &scriptPath, char *const argv[],
                      char *const envp[]);
    static void safeClose(int &fd);

   private:
    pid_t pid_;
    int pipeIn_[2];
    int pipeOut_[2];

    void executeChildProcess(const std::string &scriptPath, char *const argv[],
                             char *const envp[]);

    static int initializeEpoll(int pipe_fd);
    static void checkChildExitStatus(int status);
    static std::string getScriptDirectory(const std::string &scriptPath);
    static std::string getScriptBasename(const std::string &scriptPath);
};
