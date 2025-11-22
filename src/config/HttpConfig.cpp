#include "HttpConfig.h"

CommonConfig::CommonConfig()
    : client_max_body_size_(CommonConfig::INVALID_NUM) {}
AutoIndexDirective::AutoIndexDirective() : is_set_(false), value_(false) {}
RootDirective::RootDirective() : is_set_(false) {}
UploadStoreDirective::UploadStoreDirective() : is_set_(false) {}

ErrorPageDirective::ErrorPageDirective()
    : override_status(CommonConfig::INVALID_NUM) {}
ErrorPageDirective::ErrorPageDirective(const std::string& t, int o)
    : target(t), override_status(o) {}

ListenDirective::ListenDirective()
    : address(DEFAULT_ADDRESS), port(DEFAULT_PORT), is_default_server(false) {}

ReturnDirective::ReturnDirective() : status(CommonConfig::INVALID_NUM) {}

LocationConfig::LocationConfig() {
    allowed_methods_.push_back(MethodGET);
    allowed_methods_.push_back(MethodHEAD);
    allowed_methods_.push_back(MethodPOST);
    allowed_methods_.push_back(MethodDELETE);
}
