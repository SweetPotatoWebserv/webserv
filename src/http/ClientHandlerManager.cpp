#include "ClientHandlerManager.h"
#include "ClientHandler.h"
#include <sys/wait.h>

void ClientHandlerManager::notifyClosed(ClientHandler* handler) {
    std::vector<ClientHandler*>::iterator handler_itr = std::find(handlers_.begin(), handlers_.end(), handler);
    if (handler_itr != handlers_.end()) {
        handlers_.erase(handler_itr);
    }
    handler->cleanup();
    delete handler;
}


void ClientHandlerManager::check_timeout_all() {
    for (std::vector<ClientHandler*>::iterator handler = handlers_.begin(); handler != handlers_.end(); ++handler) {
        (*handler)->check_request_timeout();
        (*handler)->check_cgi_timeout();
    }
}
