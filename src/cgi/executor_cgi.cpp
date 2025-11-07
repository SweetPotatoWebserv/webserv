#include "executor_cgi.h"

#include <libgen.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

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
                                    StatusCode::ServerError);
    }

    if (pipe(pipeOut_) == -1) {
        close(pipeIn_[0]);
        close(pipeIn_[1]);
        throw CgiExecutionException("Failed to create stdout pipe",
                                    StatusCode::ServerError);
    }

    pid_ = fork();
    if (pid_ == -1) {
        close(pipeIn_[0]);
        close(pipeIn_[1]);
        close(pipeOut_[0]);
        close(pipeOut_[1]);
        throw CgiExecutionException("Failed to fork", StatusCode::ServerError);
    }

    if (pid_ == 0) {
        executeChildProcess(scriptPath, argv, envp);
        exit(EXIT_FAILURE);
    } else {
        return readParentProcess(requestBody);
    }
}

std::string CgiExecutor::readParentProcess(const std::string &requestBody) {
    close(pipeIn_[0]);
    close(pipeOut_[1]);

    if (!requestBody.empty()) {
        ssize_t written =
            write(pipeIn_[1], requestBody.c_str(), requestBody.size());
        if (written < 0) {
            // ToDo error handling
            std::cerr << "CGI warning: failed to write cgi stdin\n";
        }
    }
    close(pipeIn_[1]);

    std::string cgi_output;
    char buffer[BUFSIZ];  // あとで直す
    ssize_t bytes_read;
    while ((bytes_read = read(pipeOut_[0], buffer, sizeof(buffer))) > 0) {
        cgi_output.append(buffer, bytes_read);
    }
    close(pipeOut_[0]);

    if (bytes_read == -1) {
        // ToDo error handling
        std::cerr << "CGI warning: failed to read from cgi stdout\n";
    }

    int status;
    waitpid(pid_, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        // ToDo error handlihng
        std::cerr << "CGI warning: failed to exit correctly\n";
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

    if (execve(scriptPath.c_str(), argv, envp) == -1) {
        std::cerr << "CGI Error: execve failed for " << scriptPath << "\n";
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
