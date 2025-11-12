#include "HttpParser.h"

#include <sstream>

#include "HttpException.h"

std::vector<std::string> HttpParser::split_path(
    const std::string& target_path) {
    std::vector<std::string> path;
    std::string::size_type path_end = target_path.find(QUESTION_MARK);
    path.push_back(target_path.substr(0, path_end));
    // question を飛ばすために +1 する
    path.push_back((target_path.substr(path_end + 1)));
    return path;
}

std::string::size_type HttpParser::request_line_parse(const std::string& buffer,
                                                      HttpRequest& request) {
    std::string::size_type request_line_end = buffer.find(HTTP_LINE_END);
    std::string request_line = buffer.substr(0, request_line_end);
    std::vector<std::string> request_line_vec = ::split(request_line);
    if (request_line_vec.size() != REQUEST_LINE_NUM) {
        throw HttpException(HttpStatus::BadRequest,
                            HttpStatus::reason(HttpStatus::BadRequest));
    }
    request.method_ = string_to_method(request_line_vec[0]);
    if (request.method_ == MethodUNKNOWN) {
        throw HttpException(HttpStatus::NotImplemented,
                            HttpStatus::reason(HttpStatus::NotImplemented));
    }

    // TODO パスの長さ制限をかける
    if (request_line_vec[1].find(QUESTION_MARK) == std::string::npos) {
        request.request_target_.path_ = (request_line_vec[1]);
    } else {
        std::vector<std::string> path = split_path(request_line_vec[1]);
        request.request_target_.path_ = path[0];
        request.request_target_.query_string_ = path[1];
    }

    request.version_ = request_line_vec[2];
    if (request.version_ != HTTP_VERSION) {
        throw HttpException(HttpStatus::NotImplemented,
                            HttpStatus::reason(HttpStatus::NotImplemented));
    }
    return (request_line_end + HTTP_LINE_END_LEN);
}

void HttpParser::header_section_host_parse(
    const std::vector<std::string>& header_field, HttpRequest& request) {
    HostHeader host;
    if (header_field.size() > MAX_HOST_FIELD_NUM) {
        throw HttpException(HttpStatus::BadRequest,
                            HttpStatus::reason(HttpStatus::BadRequest));
    }
    // 3であれば host, localhost, 8080
    // 2であれば host, localhost
    if (header_field.size() == HOST_ONLY) {
        request.host_ = HostHeader(header_field[1]);
    } else {
        request.host_ = HostHeader(
            header_field[1], strtoul(header_field[2].c_str(), NULL, DECIMAL));
    }
}

std::string::size_type HttpParser::header_section_parse(
    const std::string& buffer, HttpRequest& request,
    std::string::size_type header_section_start) {
    std::string::size_type header_section_end =
        buffer.find(HTTP_HEADER_END, header_section_start);
    std::string header = buffer.substr(
        header_section_start, header_section_end - header_section_start);
    std::transform(header.begin(), header.end(), header.begin(), ::tolower);

    std::vector<std::string> header_fields = split(header, HTTP_LINE_END);
    for (std::vector<std::string>::iterator header_line = header_fields.begin();
         header_line != header_fields.end(); ++header_line) {
        std::vector<std::string> header_field = split(*header_line, COLON);
        if (header_field.size() < HEADER_FIELD_NUM) {
            throw HttpException(HttpStatus::BadRequest,
                                HttpStatus::reason(HttpStatus::BadRequest));
        }
        header_field[1] = trim(header_field[1]);
        if (header_field[0] == HOST) {
            HttpParser::header_section_host_parse(header_field, request);
        }
        if (header_field[0] == CONTENT_TYPE) {
            request.header_.content_type_ = header_field[1];
        }
        if (header_field[0] == CONTENT_LENGTH) {
            request.header_.content_length_ =
                strtoul(header_field[1].c_str(), NULL, DECIMAL);
        }
        if (header_field[0] == TRANSFER_ENCODING) {
            request.header_.transfer_encoding_ = header_field[1];
        }
    }
    return (header_section_end + HTTP_HEADER_END_LEN);
}

std::string HttpParser::parse_chunked(const std::string& data) {
    std::string body;
    size_t pos = 0;

    while (true) {
        std::string::size_type line_end = data.find(HTTP_LINE_END, pos);
        if (line_end == std::string::npos) break;

        std::string size_str = data.substr(pos, line_end - pos);
        std::istringstream iss(size_str);
        size_t chunk_size = 0;
        iss >> std::hex >> chunk_size;

        if (chunk_size == 0) {
            break;
        }

        pos = line_end + HTTP_LINE_END_LEN;

        if (pos + chunk_size > data.size()) break;
        body.append(data, pos, chunk_size);
        pos += chunk_size;

        if (pos + HTTP_LINE_END_LEN <= data.size() &&
            data.substr(pos, HTTP_LINE_END_LEN) == HTTP_LINE_END) {
            pos += HTTP_LINE_END_LEN;
        } else {
            break;
        }
    }
    return body;
}

void HttpParser::body_section_parse(const std::string& buffer,
                                    HttpRequest& request,
                                    std::string::size_type header_section_end) {
    if (request.header_.transfer_encoding_ == CHUNKED) {
        std::string body = buffer.substr(header_section_end);
        request.body_ = parse_chunked(body);
    } else {
        request.body_ =
            buffer.substr(header_section_end, request.header_.content_length_);
    }
}

HttpRequest HttpParser::http_request_parse(const std::string& buffer) {
    HttpRequest request;

    // ステータスラインのパース
    std::string::size_type header_section_start =
        HttpParser::request_line_parse(buffer, request);

    // ヘッダーのパース
    std::string::size_type header_section_end =
        HttpParser::header_section_parse(buffer, request, header_section_start);

    // ボディのパース
    body_section_parse(buffer, request, header_section_end);
    return request;
}
