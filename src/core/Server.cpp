#include "Server.h"

Server::Server(Event& event, Router& router, const std::string& host,
               uint16_t port)
    : listen_(Socket::listen_tcp(host, port)), event_(event), router_(router) {}

void Server::start() {
    event_.init_listen(listen_, NULL, EPOLLIN);
    event_.run();
}

// void Server::on_acceptable() {
//     struct sockaddr_in peer_addr;
//     socklen_t peer_addr_size = sizeof(peer_addr);
//
//     accept(listen_.getfd(), reinterpret_cast<struct sockaddr*>(&peer_addr),
//     &peer_addr_size);
// }
