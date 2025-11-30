#include "ClientHandlerManager.h"

#include <sys/wait.h>

#include "ClientHandler.h"

void ClientHandlerManager::check_timeout_all() {
    for (std::vector<ClientHandler*>::iterator handler_itr = handlers_.begin(); handler_itr != handlers_.end();) {
        ClientHandler* handler = *handler_itr;
        bool remove = false;

        if (handler->should_close()) {
            remove = true;
        }
        if (!remove && handler->is_cgi_timeout()) {
            handler->handle_cgi_timeout();
            ++handler_itr;
            continue;
        }

        if (!remove && handler->is_request_timeout()) {
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
