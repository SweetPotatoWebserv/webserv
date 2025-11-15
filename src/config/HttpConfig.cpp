#include "HttpConfig.h"

CommonConfig::CommonConfig()
    : root_is_set_(false),
      autoindex_is_set_(false),
      autoindex_(false),
      upload_store_is_set_(false),
      redirect_is_set_(false),
      client_max_body_size_(-1)  // 制限なし
{}

// ErrorPageDirective のコンストラクタ実装
ErrorPageDirective::ErrorPageDirective() : override_status(-1) {}
ErrorPageDirective::ErrorPageDirective(const std::string& t, int o)
    : target(t), override_status(o) {}

// ListenDirective のコンストラクタ実装
ListenDirective::ListenDirective()
    : address(DEFAULT_ADDRESS), port(DEFAULT_PORT), is_default_server(false) {}