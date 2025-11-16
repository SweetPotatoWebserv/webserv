#include "../config/HttpConfig.h"
#include "../config/HttpConfigParser.h"
#include "../event/Event.h"
#include "../http/Router.h"
#include "Server.h"

int main(void) {
    HttpConfig config = HttpConfigParser::parse("default.conf");
    Event ev;
    Router router(config);
    Server server(ev, router);
    server.start();
}
