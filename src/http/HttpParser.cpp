#include "HttpParser.h"

#include "HttpException.h"

HttpRequest HttpParser::http_request_parse(const std::string& buffer) {
    HttpRequest request;

    // ステータスラインのパース
    size_t request_line_end = buffer.find(HTTP_LINE_END);
    std::string request_line = buffer.substr(0, request_line_end);
    std::vector<std::string> request_line_vec = ::split(request_line);
    request.method_ = string_to_method(request_line_vec[0]);
    if (request.method_ == MethodUNKNOWN) {
        throw HttpException(HttpStatus::NotImplemented,
                            HttpStatus::reason(HttpStatus::NotImplemented));
    }

    if (request_line_vec[1].find(QUESTION_MARK) == std::string::npos) {
        request.request_target_.path_ = (request_line_vec[1]);
    } else {
        std::vector<std::string> path =
            ::split(request_line_vec[1], QUESTION_MARK);
        request.request_target_.path_ = (path[0]);
        request.request_target_.query_string_ = (path[1]);
    }
    request.version_ = request_line_vec[2];
    if (request.version_ != HTTP_VERSION) {
        throw HttpException(HttpStatus::NotImplemented,
                            HttpStatus::reason(HttpStatus::NotImplemented));
    }

    // ヘッダーのパース
    std::size_t header_start = request_line_end + HTTP_LINE_END_LEN;
    size_t header_end = buffer.find(HTTP_HEADER_END, header_start);
    std::string header =
        buffer.substr(header_start, header_end + HTTP_LINE_END_LEN);
    std::transform(header.begin(), header.end(), header.begin(), ::tolower);
    std::vector<std::string> header_fields = split(header, HTTP_LINE_END);
    for (std::vector<std::string>::iterator header_line = header_fields.begin();
         header_line != header_fields.end(); ++header_line) {
        std::vector<std::string> header_field = split(*header_line, COLON);
        if (header_field[0] == HOST) {
            std::vector<std::string> host_and_port =
                split(trim(header_field[1]), COLON);
            HostHeader host;
            if (host_and_port.size() == 2) {
                if (host_and_port[1].empty()) {
                    throw HttpException(
                        HttpStatus::BadRequest,
                        HttpStatus::reason(HttpStatus::BadRequest));
                }
                host = HostHeader(
                    host_and_port[0],
                    static_cast<uint16_t>(std::strtol(
                        host_and_port[1].c_str(), NULL, DECIMAL)));  // NOLINT
            } else {
                host = HostHeader(host_and_port[0]);
            }
            request.host_ = host;
        }
    }
    return request;
}
