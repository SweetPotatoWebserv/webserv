#include "HttpDate.h"

std::string HttpDate::getCurrentGMT() {
    std::time_t now = std::time(NULL);
    std::tm* gmt_time = std::gmtime(&now);
    if (gmt_time == NULL) {
        std::cerr << "Failed to get GMT time\n";
        return "";
    }

    char buffer[BUFFER_SIZE];
    if (!std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT",
                       gmt_time)) {
        std::cerr << "Failed to format GMT time\n";
        return "";
    }
    return std::string(buffer);
}
