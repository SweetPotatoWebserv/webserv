#include "response_cgi.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "../core/Common.h"
#include "../http/HttpParser.h"

namespace {
const char* const LfLf = "\n\n";
const size_t LfLfLength = 2;

const char* const StatusHeaderLower = "status";
const char* const ContentTypeHeaderLower = "content-type";
const char* const LocationHeaderLower = "location";

const int min_status = 100;
const int max_status = 599;
}  // namespace

void parseCgiResponse(HttpResponse& response, const std::string& raw_output) {
    std::string::size_type pos = raw_output.find(LfLf);

    std::string header_str;
    std::string::size_type body_start_pos;

    if (pos != std::string::npos) {
        header_str = raw_output.substr(0, pos);
        body_start_pos = pos + LfLfLength;
        response.body_ = raw_output.substr(body_start_pos);
    } else {
        // CGIスクリプトがプロトコル違反の応答をした
        response.status_code_ =
            HttpStatus::InternalServerError;  // BadGateway is better 502
        response.body_ = "CGI script returned malformed response";
        return;
    }

    std::stringstream ss(header_str);
    std::string line;

    response.status_code_ = HttpStatus::OK;

    while (std::getline(ss, line)) {
        if (line.empty()) {
            continue;
        }

        std::string::size_type colon_pos = line.find(':');

        if (colon_pos == std::string::npos || colon_pos == 0) {
            continue;
        }

        std::string header_name = line.substr(0, colon_pos);
        std::string header_value = line.substr(colon_pos + 1);

        std::string::size_type value_start =
            header_value.find_first_not_of(" \t");
        if (value_start != std::string::npos) {
            header_value = header_value.substr(value_start);
        } else {
            header_value = "";
        }

        std::transform(
            header_name.begin(), header_name.end(), header_name.begin(),
            ::tolower);  // 大文字小文字の両方に対応するため(区別される)

        if (header_name == StatusHeaderLower) {
            std::stringstream ss_status(header_value);
            int parsed_status = 0;

            if (ss_status >> parsed_status) {
                if (parsed_status >= min_status &&
                    parsed_status <= max_status) {
                    response.status_code_ = parsed_status;
                }
            }
            // else: 変換出来無い場合はデフォルト(200)のまま

        } else if (header_name == ContentTypeHeaderLower) {
            response.header_.content_type_ = header_value;

        } else if (header_name == LocationHeaderLower) {
            response.location_ = header_value;
        }
    }
}
