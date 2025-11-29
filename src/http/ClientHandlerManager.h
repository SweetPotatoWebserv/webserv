#pragma once

#include <vector>
#include "../event/Event.h"

class ClientHandler;
class CgiSession;
class ClientHandlerManager {
    public:
        ClientHandlerManager();
        ~ClientHandlerManager();
        void check_timeout_all();
        void notifyClosed(ClientHandler* handler);

    private:
        std::vector<ClientHandler*> handlers_;
        Event& event_;
        void check_cgi_timeout(CgiSession& cgi_session);
        std::vector<ClientHandler> client_handlers_;
};
