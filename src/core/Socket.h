#pragma once

#include <sys/socket.h>

#include <string>

#include "Common.h"
#include "Fd.h"

namespace fd {
class Socket : public Fd {
   private:
    static const int SOCKET_TYPE = SOCK_STREAM;
    static const int SOCKET_PROTOCOL = 0;
    static const int SOCKET_BACKLOG = 128;

   public:
    static const int SOCKET_DOMAIN = AF_INET;
    Socket();
    Socket(const Socket&);
    ~Socket();

    static Socket listen_tcp(const std::string& host, uint16_t port);
    static void set_nonblocking(int fd);

   private:
    Socket& operator=(const Socket&);
};
}  // namespace fd
