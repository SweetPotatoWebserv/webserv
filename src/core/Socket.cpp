#include "Socket.h"

#include <netdb.h>

#include <cstring>
#include <stdexcept>

#include "Common.h"
#include "Fd.h"

fd::Socket::Socket() {
    this->fd_ = ::socket(SOCKET_DOMAIN, SOCKET_TYPE, SOCKET_PROTOCOL);
    if (fd_ == -1) {
        throw std::runtime_error(
            std::string("socket failed: ").append(strerror(errno)));
    }
}

fd::Socket::Socket(const Socket& other) : Fd(other) {}

fd::Socket::~Socket() {}

fd::Socket fd::Socket::listen_tcp(const std::string& host, uint16_t port) {
    fd::Socket server_fd;
    sockaddr_in addr = {};
    if (!HostHeader::resolve_ipv4(host, port, addr)) {
        throw std::runtime_error("resolve_ipv4 failed");
    }

    int yes = 1;
    if (setsockopt(server_fd.fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
        0) {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

    if (::bind(server_fd.fd_, reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) == -1) {
        throw std::runtime_error(
            std::string("bind failed: ").append(strerror(errno)));
    }
    if (::listen(server_fd.fd_, SOCKET_BACKLOG) == -1) {
        throw std::runtime_error(
            std::string("listen failed: ").append(strerror(errno)));
    }
    return server_fd;
}

void fd::Socket::set_nonblocking(int fd) {
    if (fd == fd::Fd::DEFAULT_FD) return;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error(
            std::string("fcntl failed: ").append(strerror(errno)));
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error(
            std::string("fcntl failed: ").append(strerror(errno)));
    }
}
