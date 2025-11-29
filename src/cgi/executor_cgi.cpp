#include "executor_cgi.h"

#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "../core/Common.h"
#include "../core/Socket.h"

namespace {
const int CGI_TIMEOUT_MS = 5000;
const int MAX_EPOLL_EVENTS = 1;
const int BUFFER_SIZE = 4096;
}  // namespace

CgiExecutor::CgiExecutor() {
    pipeIn_[0] = -1;
    pipeIn_[1] = -1;
    pipeOut_[0] = -1;
    pipeOut_[1] = -1;
}

CgiExecutor::~CgiExecutor() {
    safeClose(pipeIn_[0]);
    safeClose(pipeIn_[1]);
    safeClose(pipeOut_[0]);
    safeClose(pipeOut_[1]);
}

CgiResult CgiExecutor::execute(const std::string &scriptPath,
                               char *const argv[], char *const envp[]) {
    if (pipe(pipeIn_) == -1) {
        throw CgiExecutionException("Failed to create stdin pipe",
                                    HttpStatus::InternalServerError);
    }

    if (pipe(pipeOut_) == -1) {
        safeClose(pipeIn_[0]);
        safeClose(pipeIn_[1]);
        throw CgiExecutionException("Failed to create stdout pipe",
                                    HttpStatus::InternalServerError);
    }

    fd::Socket::set_nonblocking(pipeIn_[1]);   // 親が書き込む方
    fd::Socket::set_nonblocking(pipeOut_[0]);  // 親が読み込む方

    pid_ = fork();
    if (pid_ == -1) {
        safeClose(pipeIn_[0]);
        safeClose(pipeIn_[1]);
        safeClose(pipeOut_[0]);
        safeClose(pipeOut_[1]);
        throw CgiExecutionException("Failed to fork",
                                    HttpStatus::InternalServerError);
    }

    if (pid_ == 0) {
        executeChildProcess(scriptPath, argv, envp);
        exit(EXIT_FAILURE);
    }

    safeClose(pipeIn_[0]);
    safeClose(pipeOut_[1]);
    CgiResult result;
    result.pid = pid_;
    result.readFd = pipeOut_[0];
    result.writeFd = pipeIn_[1];

    pipeIn_[1] = -1;
    pipeOut_[0] = -1;
    return result;
}

int CgiExecutor::initializeEpoll(int pipe_fd) {
    int epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        safeClose(pipe_fd);
        throw CgiExecutionException("epoll_create failed",
                                    HttpStatus::InternalServerError);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    ev.data.fd = pipe_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fd, &ev) == -1) {
        safeClose(pipe_fd);
        safeClose(epoll_fd);
        throw CgiExecutionException("epoll_ctl failed",
                                    HttpStatus::InternalServerError);
    }
    return epoll_fd;
}

void CgiExecutor::executeChildProcess(const std::string &scriptPath,
                                      char *const argv[], char *const envp[]) {
    safeClose(pipeIn_[1]);
    if (dup2(pipeIn_[0], STDIN_FILENO) == -1) {
        std::cerr << "CGI Error: dup2 failed for stdin\n";
        safeClose(pipeIn_[0]);
        safeClose(pipeOut_[0]);
        safeClose(pipeOut_[1]);
        return;
    }
    safeClose(pipeIn_[0]);

    safeClose(pipeOut_[0]);
    if (dup2(pipeOut_[1], STDOUT_FILENO) == -1) {
        std::cerr << "CGI Error: dup2 failed for stdout\n";
        safeClose(pipeOut_[1]);
        return;
    }
    safeClose(pipeOut_[1]);

    std::string dir = getScriptDirectory(scriptPath);
    if (chdir(dir.c_str()) == -1) {
        std::cerr << "CGI Error: chdir failed for " << scriptPath << "\n";
        return;
    }

    std::string basename = getScriptBasename(scriptPath);

    if (execve(basename.c_str(), argv, envp) == -1) {
        std::cerr << "CGI Error: execve failed for " << basename
                  << ". errno: " << strerror(errno) << "\n";

        exit(EXIT_FAILURE);
    }
}

void CgiExecutor::checkChildExitStatus(int status) {
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0) {
            return;
        }
        throw CgiExecutionException("CGI script error",
                                    HttpStatus::InternalServerError);
    }
    throw CgiExecutionException("CGI crashed", HttpStatus::InternalServerError);
}

void CgiExecutor::safeClose(int &fd) {
    if (fd == -1) {
        return;
    }

    int tmp_fd = fd;
    fd = -1;

    if (close(tmp_fd) == -1) {
        std::stringstream ss;
        ss << "Failed CgiExecutor close(" << tmp_fd << ")";

        perror(ss.str().c_str());
    }
}

std::string CgiExecutor::getScriptDirectory(const std::string &scriptPath) {
    std::string::size_type pos = scriptPath.rfind('/');

    if (pos == std::string::npos) {
        return ".";
    }

    // スラッシュが先頭にある場合ルートを返す
    if (pos == 0) {
        return "/";
    }

    return scriptPath.substr(0, pos);
}

std::string CgiExecutor::getScriptBasename(const std::string &scriptPath) {
    std::string::size_type pos = scriptPath.rfind('/');
    std::string basename;
    if (pos == std::string::npos) {
        basename = scriptPath;
    } else {
        basename = scriptPath.substr(pos + 1);
    }
    return "./" + basename;
}
