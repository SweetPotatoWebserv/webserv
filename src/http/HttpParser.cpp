#include "HttpParser.h"

#include <unistd.h>

#include <sstream>

std::vector<std::string> split(const std::string& buffer,
                               const std::string& sep = " ") {
    std::vector<std::string> res;
    std::string::size_type offset = 0;
    size_t sep_len = sep.size();
    while (1) {
        size_t pos = buffer.find(sep, offset);
        if (pos == std::string::npos) {
            res.push_back(buffer.substr(offset));
            break;
        }
        res.push_back(buffer.substr(offset, pos - offset));
        offset = pos + sep_len;
    }
    return res;
}

HttpRequest HttpParser::http_request_parse(const std::string& buffer) {
    HttpRequest request;

    // ステータスラインのパース
    std::string::size_type status_line_end = buffer.find("\r\n");
    std::string status_line_str = buffer.substr(0, status_line_end + 2);
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
    size_t header_end = buffer.find("\r\n\r\n", status_line_end + 2);
    std::string header_str = buffer.substr(status_line_end + 2, header_end + 2);
    std::vector<std::string> header_vec = split(header_str, "\r\n");
    for (std::vector<std::string>::iterator it = header_vec.begin();
         it != header_vec.end(); ++it) {
        // TODO 他のヘッダーも対応する
        // 全て小文字に変換する
        // 定数に置き換える
        std::vector<std::string> header = split(*it, ": ");
        // TODO localhost は 127.0.0.1 に統一する
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

ssize_t HttpResponse::send_response(int client_fd, HttpResponse& response) {
    std::ostringstream oss;

    oss << HTTP_VERSION << " " << response.status_code_ << " "
        << response.message_ << "\r\n";
    // if (response.date_.to_string().size()) // NOLINT
    //     oss << "Date: " << response.date_.to_string() << "\r\n";
    // if (response.header_.content_type_.size()) // NOLINT
    //     oss << "Content-Type: " << response.header_.content_type_ << "\r\n";

    oss << "Content-Length: " << response.body_.size() << "\r\n";

    // if (!response.location_.empty())
    //     oss << "Location: " << response.location_ << "\r\n";

    oss << "\r\n";

    oss << response.body_;

    std::string response_str = oss.str();

    ssize_t sent = write(client_fd, response_str.c_str(), response_str.size());
    return sent;
}
