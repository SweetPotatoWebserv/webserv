#include "String.h"

#include <ctype.h>

#include <cctype>

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
    if (s.empty()) return "";
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

bool search_header_field(const std::string& request_message,
                         const std::string& search_field,
                         std::vector<std::string>& found_field_vec) {
    std::string::size_type search_header_start =
        request_message.find(search_field);
    if (search_header_start == std::string::npos) {
        return false;
    }
    std::string::size_type search_header_end =
        request_message.find(HTTP_LINE_END, search_header_start);
    if (search_header_end == std::string::npos) {
        return false;
    }

    std::string found_field = request_message.substr(
        search_header_start, search_header_end - search_header_start);
    found_field_vec = split(found_field, COLON);
    if (found_field_vec.size() != HEADER_FIELD_NUM) return false;
    if (found_field_vec[1].empty()) return false;

    found_field_vec[1] = trim(found_field_vec[1]);
    return true;
}
