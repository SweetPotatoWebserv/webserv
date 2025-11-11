#include "executor_cgi.h"

#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "../core/Common.h"

namespace {
const int CGI_TIMEOUT_MS = 5000;
const int MAX_EPOLL_EVENTS = 1;
const int BUFFER_SIZE = 100000;
}  // namespace

CgiExecutor::CgiExecutor() {
    pipeIn_[0] = -1;
    pipeIn_[1] = -1;
    pipeOut_[0] = -1;
    pipeOut_[1] = -1;
}

CgiExecutor::~CgiExecutor() {
    if (pipeIn_[0] != -1) close(pipeIn_[0]);
    if (pipeIn_[1] != -1) close(pipeIn_[1]);
    if (pipeOut_[0] != -1) close(pipeOut_[0]);
    if (pipeOut_[1] != -1) close(pipeOut_[1]);
}

std::string CgiExecutor::execute(const std::string &scriptPath,
                                 char *const argv[], char *const envp[],
                                 const std::string &requestBody) {
    if (pipe(pipeIn_) == -1) {
        throw CgiExecutionException("Failed to create stdin pipe",
                                    HttpStatus::InternalServerError);
    }

    if (pipe(pipeOut_) == -1) {
        close(pipeIn_[0]);
        close(pipeIn_[1]);
        throw CgiExecutionException("Failed to create stdout pipe",
                                    HttpStatus::InternalServerError);
    }

    pid_ = fork();
    if (pid_ == -1) {
        close(pipeIn_[0]);
        close(pipeIn_[1]);
        close(pipeOut_[0]);
        close(pipeOut_[1]);
        throw CgiExecutionException("Failed to fork",
                                    HttpStatus::InternalServerError);
    }

    if (pid_ == 0) {
        executeChildProcess(scriptPath, argv, envp);
        exit(EXIT_FAILURE);
    }

    close(pipeIn_[0]);
    close(pipeOut_[1]);
    try {
        return readParentProcess(requestBody);
    } catch (const std::exception &e) {
        waitpid(
            pid_, NULL,
            0);  // epoll_createなどで失敗した場合にも子プロセスをきちんと削除するため
        throw;   // 例外をそのまま再スロー
    }
}

std::string CgiExecutor::readParentProcess(const std::string &requestBody) {
    if (!requestBody.empty()) {
        ssize_t written =
            write(pipeIn_[1], requestBody.c_str(), requestBody.size());
        if (written < 0) {
            std::cerr << "CGI warning: failed to write cgi stdin\n";
        }
    }
    close(pipeIn_[1]);

    int epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        close(pipeOut_[0]);
        throw CgiExecutionException("epoll_create failed",
                                    HttpStatus::InternalServerError);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = pipeOut_[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipeOut_[0], &ev) == -1) {
        close(pipeOut_[0]);
        close(epoll_fd);
        throw CgiExecutionException("epoll_ctl failed",
                                    HttpStatus::InternalServerError);
    }

    std::string cgi_output;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    bool timeout_occured = false;

    while (true) {
        int num_events =
            epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, CGI_TIMEOUT_MS);

        if (num_events == -1) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "CGI Error: epoll_wait failed\n";
            break;
        }

        if (num_events == 0) {
            timeout_occured = true;
            kill(pid_, SIGKILL);
            std::cerr << "CGI Error: script timed out\n";
            break;
        }

        if (events[0].events & EPOLLIN) {
            char buffer[BUFFER_SIZE];
            ssize_t bytes_read = read(pipeOut_[0], buffer, sizeof(buffer));

            if (bytes_read > 0) {
                cgi_output.append(buffer, bytes_read);
            } else if (bytes_read == 0) {
                break;
            } else {
                std::cerr << "CGI warning: failed to read from cgi stdout\n";
                break;
            }
        }
        // EPOLLIN が無かった場合のみ、HUP や ERR をチェックする
        else if (events[0].events & (EPOLLHUP | EPOLLERR)) {
            std::cerr << "CGI warning: EPOLLHUP/EPOLLERR without EPOLLIN.\n";
            break;
        }
    }

    close(epoll_fd);
    close(pipeOut_[0]);

    int status;
    waitpid(pid_, &status, 0);

    if (timeout_occured) {
        throw CgiExecutionException("CGI script timed out",
                                    HttpStatus::RequestTimeout);
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        std::cerr << "CGI warning: child process failed (Status: "
                  << WEXITSTATUS(status) << ")\n";
        throw CgiExecutionException("CGI script execution failed",
                                    HttpStatus::InternalServerError);
    }

    return cgi_output;
}

void CgiExecutor::executeChildProcess(const std::string &scriptPath,
                                      char *const argv[], char *const envp[]) {
    close(pipeIn_[1]);
    if (dup2(pipeIn_[0], STDIN_FILENO) == -1) {
        std::cerr << "CGI Error: dup2 failed for stdin\n";
        close(pipeIn_[0]);
        close(pipeOut_[0]);
        close(pipeOut_[1]);
        return;
    }
    close(pipeIn_[0]);

    close(pipeOut_[0]);
    if (dup2(pipeOut_[1], STDOUT_FILENO) == -1) {
        std::cerr << "CGI Error: dup2 failed for stdout\n";
        close(pipeIn_[0]);
        close(pipeIn_[1]);
        close(pipeOut_[1]);
        return;
    }
    close(pipeOut_[1]);

    std::string dir = getScriptDirectory(scriptPath);
    if (chdir(dir.c_str()) == -1) {
        std::cerr << "CGI Error: chdir failed for " << scriptPath << "\n";
        return;
    }

    std::string basename = getScriptBasename(scriptPath);

    if (execve(basename.c_str(), argv, envp) == -1) {
        std::cerr << "CGI Error: execve failed for " << basename
                  << ". errno: " << strerror(errno) << "\n";
    }
}

std::string CgiExecutor::getScriptDirectory(const std::string &scriptPath) {
    char *path_c_str = strdup(scriptPath.c_str());
    if (path_c_str == NULL) {
        // ここではルート返して、child processのexecutorでエラー
        return ".";
    }

    char *dir = dirname(path_c_str);
    std::string dir_str(dir);
    free(path_c_str);
    return dir_str;
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
