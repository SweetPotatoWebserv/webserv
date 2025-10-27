#include "Server.h"

Socket::Socket() {
    fd_ = ::socket(SOCKET_DOMAIN, SOCKET_TYPE, SOCKET_PROTOCOL);
    if (fd_ < 0) {
        throw std::runtime_error("socket failed: " +
                                 std::string(strerror(errno)));
    }
}

Socket::~Socket() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

Socket Socket::listen_tcp(const std::string& host, uint16_t port) {
    Socket server_fd;
    sockaddr_in addr = {};
    if (HostHeader::resolve_ipv4(host, port, addr)) {
        std::cerr << "resolve_ipv4 failed";
    }
    if (::bind(server_fd.fd_, (sockaddr*)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error("bind failed: " +
                                 std::string(strerror(errno)));
    }
    if (::listen(server_fd.fd_, SOCKET_BACKLOG) == -1) {
        throw std::runtime_error("listen failed: " +
                                 std::string(strerror(errno)));
    }

    return server_fd;
}
