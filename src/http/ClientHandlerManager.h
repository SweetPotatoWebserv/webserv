#pragma once

#include <vector>

#include "../event/Event.h"

class ClientHandler;
class ClientHandlerManager {
   private:
    std::vector<ClientHandler*> handlers_;

   public:
    void add(ClientHandler* handler) { handlers_.push_back(handler); }
    const std::vector<ClientHandler*>& getHandlers() const { return handlers_; }

    void check_timeout_all();

    ClientHandlerManager(){};
    ~ClientHandlerManager();
};
