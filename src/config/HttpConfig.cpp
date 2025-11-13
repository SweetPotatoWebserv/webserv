#include "HttpConfig.h"  // 自分のヘッダーをインクルード

CommonConfig::CommonConfig()
    : root_is_set(false),
      autoindex_is_set(false),
      autoindex(false),
      upload_store_is_set(false),
      redirect_is_set(false),
      client_max_body_size(-1)  // 制限なし
{}

// ErrorPageDirective のコンストラクタ実装
ErrorPageDirective::ErrorPageDirective() : override_status(-1) {}
ErrorPageDirective::ErrorPageDirective(std::string& t, int o)
    : target(t), override_status(o) {}

// ListenDirective のコンストラクタ実装
ListenDirective::ListenDirective()
    : port(DEFAULT_PORT), is_default_server(false) {}