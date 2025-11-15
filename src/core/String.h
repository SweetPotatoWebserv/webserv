#pragma once

#include <cctype>
#include <string>
#include <vector>

#include "../core/Common.h"

std::vector<std::string> split(const std::string& buffer,
                               const std::string& sep = " ");
std::string trim(const std::string& s);
bool search_header_field(const std::string& request_message,
                         const std::string& search_field,
                         std::vector<std::string>& found_field_vec);
