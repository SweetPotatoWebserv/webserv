#include "Fd.h"

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "fcntl.h"

Fd::Fd(const char* filename, int flags, mode_t mode) : fd_(-1) {
    fd_ = ::open(filename, flags, mode);
    if (fd_ == -1) {
        throw std::runtime_error("open() failed" +
                                 std::string(strerror(errno)));
    }
}

int Fd::getFd() const { return fd_; }

std::vector<char> Fd::FreadAll() const {
    std::vector<char> result;
    char buf[DEFAULT_BUFFER_SIZE];

    while (true) {
        ssize_t len = ::read(fd_, buf, DEFAULT_BUFFER_SIZE);
        if (len == -1) {
            throw std::runtime_error("read() failed: " +
                                     std::string(strerror(errno)));
        }
        if (len == 0) break;
        result.insert(result.end(), buf, buf + len);
    }
    return result;
}

void Fd::FwriteAll(const std::string& buf) const {
    std::string::size_type total_written = 0;
    std::string::size_type buf_size = buf.size();
    while (total_written < buf_size) {
        ssize_t len = write(fd_, buf.c_str() + total_written, buf_size - total_written);
        if (len == -1)
            throw std::runtime_error("write() failed: " +
                                     std::string(strerror(errno)));
        if (len == 0)
            throw std::runtime_error("write() returned 0 unexpectedly");
        total_written += static_cast<std::string::size_type>(len);
    }
}

Fd::~Fd() {
    if (fd_ != -1) {
        ::close(fd_);
    }
}
