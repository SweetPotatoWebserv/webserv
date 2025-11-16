#include "HttpConfig.h"

// ErrorPageDirective のコンストラクタ実装
ErrorPageDirective::ErrorPageDirective() : override_status(-1) {}
ErrorPageDirective::ErrorPageDirective(const std::string& t, int o)
    : target(t), override_status(o) {}

// ListenDirective のコンストラクタ実装
ListenDirective::ListenDirective()
    : address(DEFAULT_ADDRESS), port(DEFAULT_PORT), is_default_server(false) {}
