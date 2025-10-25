#include "common.h"

int main(void) {
    Server server;
    try {
        server.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
