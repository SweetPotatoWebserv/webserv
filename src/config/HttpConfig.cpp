#include "HttpConfig.h"

CommonConfig::CommonConfig() : autoindex_(false), client_max_body_size_(0) {}

ErrorPageDirective::ErrorPageDirective() : override_status(-1) {}
ErrorPageDirective::ErrorPageDirective(std::string& t, int o)
    : target(t), override_status(o) {}

ListenDirective::ListenDirective()
    : address(DEFAULT_ADDRESS), port(DEFAULT_PORT), is_default_server(false) {}
