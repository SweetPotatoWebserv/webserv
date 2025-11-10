#include "String.h"

std::vector<std::string> split(const std::string& buffer,
                               const std::string& sep) {
    std::vector<std::string> res;
    std::string::size_type offset = 0;
    std::string::size_type sep_len = sep.size();
    while (1) {
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
