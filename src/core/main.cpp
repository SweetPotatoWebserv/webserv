#include "../event/Event.h"
#include "../http/Router.h"
#include "Server.h"

int main(void) {
    Event ev;
    Router router;
    Server server(ev, router);
    server.start();
}
