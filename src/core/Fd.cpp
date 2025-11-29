#include "Fd.h"

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "fcntl.h"

fd::Fd::Fd(const char* filename, int flags, mode_t mode) : fd_(Fd::DEFAULT_FD) {
    fd_ = ::open(filename, flags, mode);
    if (fd_ == Fd::DEFAULT_FD) {
        throw fd::Exception("Failed to open file '" + std::string(filename) + "': " + std::string(strerror(errno)));
    }
}

int fd::Fd::getFd() const { return fd_; }

void fd::Fd::readAll(std::vector<char>& response) const {
    response.clear();
    char buf[Fd::DEFAULT_BUFFER_SIZE];

    while (true) {
        ssize_t len = ::read(fd_, buf, Fd::DEFAULT_BUFFER_SIZE);
        if (len == -1) {
            throw std::runtime_error(std::string("read() failed: ").append(strerror(errno)));
        }
        if (len == 0) break;
        response.insert(response.end(), buf, buf + len);
    }
}

void fd::Fd::writeAll(const std::string& buf) const {
    std::string::size_type total_written = 0;
    std::string::size_type buf_size = buf.size();
    while (total_written < buf_size) {
        ssize_t len = ::write(fd_, buf.c_str() + total_written, buf_size - total_written);
        if (len == -1)
            throw std::runtime_error(std::string("write() failed: ").append((strerror(errno))));
        if (len == 0)
            throw std::runtime_error("write() returned 0 unexpectedly");
        total_written += static_cast<std::string::size_type>(len);
    }
}

void fd::Fd::close(int fd) {
    if (fd == Fd::DEFAULT_FD)
        return;
    ::close(fd);
}

fd::Fd::~Fd() {
    Fd::close(this->fd_);
}

fd::Exception::Exception(const std::string& message)
    : std::runtime_error(message) {}
