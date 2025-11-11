#include "String.h"

std::vector<std::string> split(const std::string& buffer,
                               const std::string& sep) {
    std::vector<std::string> res;
    std::string::size_type offset = 0;
    std::string::size_type sep_len = sep.size();
    if (sep.empty()) {
        res.push_back(buffer);
        return res;
    }
    while (true) {
        std::string::size_type pos = buffer.find(sep, offset);
        if (pos == std::string::npos) {
            res.push_back(buffer.substr(offset));
            break;
        }
        res.push_back(buffer.substr(offset, pos - offset));
        offset = pos + sep_len;
    }
    return res;
}

std::string trim(const std::string& s) {
    if (s.empty()) return s;
    std::string::const_iterator start = s.begin();
    while (start != s.end() &&
           std::isspace(static_cast<unsigned char>(*start))) {  // NOLINT
        ++start;
    }
    std::string::const_iterator end = s.end();
    while (end != start &&
           std::isspace(static_cast<unsigned char>(*(end - 1)))) {  // NOLINT
        --end;
    }
    return std::string(start, end);
}
