#include "Server.h"

Server::Server(Event& event, Router& router, const std::string& host,
               uint16_t port)
    : listen_(Socket::listen_tcp(host, port)), event_(event), router_(router) {}

void Server::start() {
    // event_.add(listen_.getFd(), EPOLLIN, &on_acceptable(), this);
    event_.run(listen_);
}

void Server::on_acceptable(int fd, void* self) {
    Server* server = static_cast<Server*>(self);
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int client_fd =
        accept(fd, reinterpret_cast<struct sockaddr*>(&client), &len);
    if (client_fd == -1) return;

    Socket::set_nonblocking(client_fd);
    // ClientHandler* handler =
    //     new ClientHandler(client_fd, server->event_, server->router);
    // server->event_.add(client_fd, EPOLLIN, &ClientHandler, handler);
}
