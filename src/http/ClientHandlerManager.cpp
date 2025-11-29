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
    std::cout << handlers_.size() << '\n';
    for (std::vector<ClientHandler*>::iterator handler_itr = handlers_.begin(); handler_itr != handlers_.end();) {
        ClientHandler* handler = *handler_itr;
        bool remove = false;

        if (handler->should_close()) {
            remove = true;
        }
        if (!remove && handler->is_request_timeout()) {
            remove = true;
        }

        if (!remove && handler->is_cgi_timeout()) {
            handler->handle_cgi_timeout();
            remove = true;
        }

        if (remove) {
            handler->cleanup();
            delete handler;
            handler_itr = handlers_.erase(handler_itr);
        } else {
            handler_itr++;
        }
    }
}
