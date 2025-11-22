#include "../config/HttpConfig.h"
#include "../config/HttpConfigParser.h"
#include "../event/Event.h"
#include "../http/Router.h"
#include "Server.h"

int main(void) {
    try {
        HttpConfig config = HttpConfigParser::parse("default.conf");
        Event ev;
        Router router(config);
        const std::vector<ServerConfig>& server_configs = config.getservers();
        std::vector<Server> servers;
        size_t server_count = server_configs.size();
        servers.reserve(server_count);
        for (size_t i = 0; i < server_count; ++i) {
            servers.push_back(Server(ev, router, server_configs[i]));
        }

        for (size_t i = 0; i < server_count; ++i) {
            servers[i].start();
        }
        ev.run();
    } catch (std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
