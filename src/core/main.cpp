#include <signal.h>

#include <stdexcept>

#include "../config/HttpConfig.h"
#include "../config/HttpConfigParser.h"
#include "../event/Event.h"
#include "../http/Router.h"
#include "Server.h"

int main(int argc, char* argv[]) {
    const char* config_path = "default.conf";

    if (argc == 2) {
        config_path = argv[1];
    } else if (argc > 2) {
        std::cerr << "Error: Too many arguments.\n";
        std::cerr << "Usage: ./webserv [configuration file]\n";
        return 1;
    }

    try {
        // クライアントのコネクションが切断された場合 SIGPIPE になる
        // プロセス自体が落ちるのを防ぐために、SIGPIPE を無視する
        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
            throw std::runtime_error("signal() failed: " +
                                     std::string(strerror(errno)));
        }
        HttpConfig config = HttpConfigParser::parse(config_path);
        Event ev;
        Router router(config);
        ClientHandlerManager manager;
        const std::vector<ServerConfig>& server_configs = config.getservers();
        std::vector<Server> servers;
        size_t server_count = server_configs.size();
        servers.reserve(server_count);
        for (size_t i = 0; i < server_count; ++i) {
            servers.push_back(Server(ev, router, server_configs[i], manager));
        }

        for (size_t i = 0; i < server_count; ++i) {
            servers[i].start();
        }
        ev.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
