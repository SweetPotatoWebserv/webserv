#pragma once

#include <fcntl.h>
#include <sys/types.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

class OpenException : public std::runtime_error {
   public:
    explicit OpenException(const std::string& message);
};

class Fd {
   public:
    Fd(const char* filename, int flags, mode_t mode = DEFAULT_MODE);
    ~Fd();
    int getFd() const;
    std::vector<char> FreadAll() const;
    void FwriteAll(const std::string&) const;

   private:
    Fd();
    Fd(const Fd&);
    Fd& operator=(const Fd&);
    int fd_;
    static const int DEFAULT_BUFFER_SIZE = 4096;
    static const mode_t DEFAULT_MODE = 0644;
};
