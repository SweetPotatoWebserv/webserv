#pragma once

#include <stdexcept>

namespace fd {
class Exception : public std::runtime_error {
   public:
    explicit Exception(const std::string& message);
};
}  // namespace fd
