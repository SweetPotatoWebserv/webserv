#include "Server.h"

#include "../http/ClientHandler.h"

Server::Server(Event& event, Router& router, const ServerConfig& server_config)
    : listen_(fd::Socket::listen_tcp(server_config.getListens().address,
                                     server_config.getListens().port)),
      event_(event),
      router_(router),
      server_config_(server_config) {}

void Server::start() {
    event_.add(listen_.getFd(), EPOLLIN,
               reinterpret_cast<EventCallback>(on_acceptable), this);
}

void Server::on_acceptable(int fd, uint32_t event, void* self) {  // NOLINT
    Server* server = static_cast<Server*>(self);
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int client_fd =
        accept(fd, reinterpret_cast<struct sockaddr*>(&client), &len);
    if (client_fd == -1) return;
    std::cout << "accepted\n";
    fd::Socket::set_nonblocking(client_fd);
    ClientHandler* handler = new ClientHandler(
        client_fd, server->event_, server->router_, server->server_config_);
    server->event_.add(client_fd, event,
                       reinterpret_cast<EventCallback>(ClientHandler::on_event),
                       handler);
}
