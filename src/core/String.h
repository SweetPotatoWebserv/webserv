#pragma once

#include <string>
#include <vector>

std::vector<std::string> split(const std::string& buffer,
                               const std::string& sep = " ");
std::string trim(const std::string& s);
