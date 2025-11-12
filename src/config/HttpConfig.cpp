#include "HttpConfig.h" // 自分のヘッダーをインクルード

// ErrorPageDirective のコンストラクタ実装
ErrorPageDirective::ErrorPageDirective(): override_status(-1) {}

// ListenDirective のコンストラクタ実装
ListenDirective::ListenDirective() : port(DEFAULT_PORT), is_default_server(false) {}