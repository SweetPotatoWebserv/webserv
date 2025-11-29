#pragma once

#include <fcntl.h>
#include <sys/types.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace fd {
class Fd {
   private:
    static const int DEFAULT_BUFFER_SIZE = 4096;
    static const mode_t DEFAULT_MODE =
        0777;  // umask
               // で制限をかけることを前提とするため、フルアクセスに設定する

   protected:
    static const int DEFAULT_FD = -1;

    int fd_;  // NOLINT

   public:
    static void close(int fd);

    Fd();
    Fd(const Fd&);
    Fd(const char* filename, int flags, mode_t mode = DEFAULT_MODE);
    ~Fd();

    int getFd() const;
    void readAll(std::vector<char>& response) const;
    void writeAll(const std::string&) const;

   private:
    Fd& operator=(const Fd&);
};
}  // namespace fd
