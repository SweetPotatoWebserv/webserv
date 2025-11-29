#pragma once

#include <vector>
#include "ClientHandler.h"
#include "../event/Event.h"

class ClientHandlerManager {
    public:
        check_reponse_timeout();
        void check_timeout_all();
        void notifyClosed(ClientHandler* handler);

    private:
        std::vector<ClientHandler*> handlers_;
        Event& event_;
        void check_cgi_timeout(CgiSession& cgi_session);
        std::vector<ClientHandler> client_handlers_;
        static const int CGI_TIMEOUT_SEC = 5;
};
