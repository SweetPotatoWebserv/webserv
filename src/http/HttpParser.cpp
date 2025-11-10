#include "HttpParser.h"

HttpRequest HttpParser::http_request_parse(const std::string& buffer) {
    HttpRequest request;

    // ステータスラインのパース
    size_t status_line_end = buffer.find(HTTP_LINE_END);
    std::string status_line_str =
        buffer.substr(0, status_line_end + HTTP_LINE_END_LEN);
    std::vector<std::string> status_line_vec = ::split(status_line_str);
    request.method_ = string_to_method(status_line_vec[0]);

    if (status_line_vec[1].find('?') == std::string::npos) {
        request.request_target_.path_ = (status_line_vec[1]);
    } else {
        std::vector<std::string> path = ::split(status_line_vec[1], "?");
        request.request_target_.path_ = (path[0]);
        request.request_target_.query_string_ = (path[1]);
    }

    // ヘッダーのパース
    size_t header_end =
        buffer.find(HTTP_HEADER_END, status_line_end + HTTP_LINE_END_LEN);
    std::string header_str = buffer.substr(status_line_end + HTTP_LINE_END_LEN,
                                           header_end + HTTP_LINE_END_LEN);
    std::vector<std::string> header_vec = split(header_str, "\r\n");
    for (std::vector<std::string>::iterator it = header_vec.begin();
         it != header_vec.end(); ++it) {
        // TODO 他のヘッダーも対応する
        std::vector<std::string> header = split(*it, ": ");
        if (header[0] == "Host") {
            std::vector<std::string> host_and_port = split(header[1], ":");
            HostHeader host(host_and_port[0], static_cast<uint16_t>(std::strtol(
                                                  host_and_port[1].c_str(),
                                                  NULL, 10)));  // NOLINT
            request.host_ = host;
        }
    }
    return request;
}
