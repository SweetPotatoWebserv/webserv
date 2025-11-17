#include "HttpConfig.h"

CommonConfig::CommonConfig()
    : client_max_body_size_(CommonConfig::INVALID_NUM) {}
AutoIndexDirective::AutoIndexDirective() : is_set_(false), value_(false) {}
ReturnDirective::ReturnDirective() : status(CommonConfig::INVALID_NUM) {}
RootDirective::RootDirective() : is_set_(false) {}
UploadStoreDirective::UploadStoreDirective() : is_set_(false) {}

ErrorPageDirective::ErrorPageDirective()
    : override_status(CommonConfig::INVALID_NUM) {}
ErrorPageDirective::ErrorPageDirective(const std::string& t, int o)
    : target(t), override_status(o) {}

ListenDirective::ListenDirective()
    : address(DEFAULT_ADDRESS), port(DEFAULT_PORT), is_default_server(false) {}
