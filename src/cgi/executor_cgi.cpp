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

namespace {
const int CGI_TIMEOUT_MS = 5000;
const int MAX_EPOLL_EVENTS = 1;
const int BUFFER_SIZE = 4096;

const int EXIT_CODE_PERMISSION_DENIED = 126;
const int EXIT_CODE_COMMAND_NOT_FOUND = 127;

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

void CgiExecutor::writeAll(int fd, const char *buffer, size_t size) {
    size_t total_written = 0;
    while (total_written < size) {
        ssize_t written =
            write(fd, buffer + total_written, size - total_written);

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "CGI warning: failed to write cgi stdin fully. errno: "
                      << strerror(errno) << "\n";
            return;
        }

        total_written += written;
    }
}

std::string CgiExecutor::execute(const std::string &scriptPath,
                                 char *const argv[], char *const envp[],
                                 const std::string &requestBody) {
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
    try {
        return readParentProcess(requestBody);
    } catch (const std::exception &e) {
        waitpid(
            pid_, NULL,
            0);  // epoll_createなどで失敗した場合にも子プロセスをきちんと削除するため
        throw;   // 例外をそのまま再スロー
    }
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

std::string CgiExecutor::readParentProcess(const std::string &requestBody) {
    if (!requestBody.empty()) {
        writeAll(pipeIn_[1], requestBody.c_str(), requestBody.size());
    }
    safeClose(pipeIn_[1]);

    int epoll_fd = initializeEpoll(pipeOut_[0]);

    std::string cgi_output;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    bool timeout_occurred = false;

    bool hup_occurred = false;
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
            timeout_occurred = true;
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
                // 正常終了: データが尽きた。
                break;
            } else {
                std::cerr << "CGI warning: failed to read from cgi stdout\n";
                break;
            }
        }

        if (events[0].events & (EPOLLHUP | EPOLLERR)) {
            hup_occurred = true;
        }

        if (hup_occurred && !(events[0].events & EPOLLIN)) {
            break;
        }
        if (hup_occurred && !(events[0].events & EPOLLIN)) {
            break;
        }
    }

    safeClose(epoll_fd);
    safeClose(pipeOut_[0]);

    int status;
    waitpid(pid_, &status, 0);

    if (timeout_occurred) {
        throw CgiExecutionException("CGI script timed out",
                                    HttpStatus::RequestTimeout);
    }

    checkChildExitStatus(status);
    return cgi_output;
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

        if (errno == EACCES) {
            exit(EXIT_CODE_PERMISSION_DENIED);
        } else if (errno == ENOENT) {
            exit(EXIT_CODE_COMMAND_NOT_FOUND);
        } else {
            exit(EXIT_FAILURE);
        }
    }
}

void CgiExecutor::checkChildExitStatus(int status) {
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0) {
            return;
        }
        if (exit_code == EXIT_CODE_PERMISSION_DENIED) {
            throw CgiExecutionException("Permission denied",
                                        HttpStatus::Forbidden);
        }
        if (exit_code == EXIT_CODE_COMMAND_NOT_FOUND) {
            throw CgiExecutionException("Command not found",
                                        HttpStatus::NotFound);
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
