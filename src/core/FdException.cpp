#include "FdException.h"

fd::Exception::Exception(const std::string& message)
    : std::runtime_error(message) {}
