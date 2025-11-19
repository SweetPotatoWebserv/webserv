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
#include "../core/String.h"
#include "../http/HttpParser.h"

namespace {
const char* const LF_LF = "\n\n";
const size_t LF_LF_LENGTH = 2;

const char* const STATUS_HEADER_LOWER = "status";
const char* const CONTENT_TYPE_HEADER = "content-type";
const char* const LOCATION_HEADER_LOWER = "location";

const int MIN_STATUS = 100;
const int MAX_STATUS = 599;
}  // namespace

void parseCgiHeaderLine(const std::string& line, HttpResponse& response) {
    std::string::size_type colon_pos = line.find(':');

    if (colon_pos == std::string::npos || colon_pos == 0) {
        return;
    }

    std::string header_name = line.substr(0, colon_pos);
    std::string header_value = line.substr(colon_pos + 1);

    header_value = trim(header_value);
    std::transform(header_name.begin(), header_name.end(), header_name.begin(),
                   ::tolower);

    if (header_name == STATUS_HEADER_LOWER) {
        std::stringstream ss_status(header_value);
        int parsed_status = 0;

        if (ss_status >> parsed_status) {
            if (parsed_status >= MIN_STATUS && parsed_status <= MAX_STATUS) {
                response.status_code_ = parsed_status;
            }
        }
        // else: 変換出来無い場合はデフォルト(200)のまま

    } else if (header_name == CONTENT_TYPE_HEADER) {
        response.header_.content_type_ = header_value;

    } else if (header_name == LOCATION_HEADER_LOWER) {
        response.location_ = header_value;
    }
}

void parseCgiResponse(HttpResponse& response, const std::string& raw_output) {
    std::string::size_type pos = raw_output.find(LF_LF);

    std::string header_str;
    std::string::size_type body_start_pos;

    if (pos != std::string::npos) {
        header_str = raw_output.substr(0, pos);
        body_start_pos = pos + LF_LF_LENGTH;
        response.body_ = raw_output.substr(body_start_pos);
    } else {
        // CGIスクリプトがプロトコル違反の応答をした
        response.status_code_ = HttpStatus::InternalServerError;
        response.body_ = "CGI script returned malformed response";
        return;
    }

    std::stringstream ss(header_str);
    std::string line;

    response.status_code_ = HttpStatus::OK;

    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        parseCgiHeaderLine(line, response);
    }

    if (!response.location_.empty() &&
        response.status_code_ == HttpStatus::OK) {
        response.status_code_ = HttpStatus::Found;
    }

    if (response.status_code_ == HttpStatus::NoContent) {
        return;
    }

    if ((response.status_code_ == HttpStatus::OK ||
         response.status_code_ == HttpStatus::Created) &&
        response.location_.empty() &&  // リダイレクトでもない
        response.header_.content_type_.empty())  // Content-Typeが空
    {
        // レスポンスが不正なので 500に上書きする
        response.status_code_ = HttpStatus::InternalServerError;
        response.body_ =
            "CGI script returned malformed response (missing Content-Type)";
    }
}
