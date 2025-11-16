#include "HttpResponse.h"

#include "Router.h"

ssize_t HttpResponse::send_response(int client_fd, HttpResponse& response) {
    std::ostringstream oss;

    oss << HTTP_VERSION << " " << response.status_code_ << " "
        << response.message_ << "\r\n";
    oss << "Date: " << HttpDate::getCurrentGMT() << "\r\n";
    if (!response.header_.content_type_.empty())  // NOLINT
        oss << "Content-Type: " << response.header_.content_type_ << "\r\n";

    oss << "Content-Length: " << response.header_.content_length_ << "\r\n";

    if (!response.location_.empty())
        oss << "Location: " << response.location_ << "\r\n";

    oss << "\r\n";

    std::string header = oss.str();
    const std::string& body = response.body_;

    size_t total = header.size();
    ssize_t sent = 0;
    while (sent < static_cast<ssize_t>(total)) {
        ssize_t len = write(client_fd, header.c_str() + sent, total - sent);
        if (len == -1) return -1;
        sent += len;
    }

    total = body.size();
    sent = 0;
    while (sent < static_cast<ssize_t>(total)) {
        ssize_t len = write(client_fd, body.c_str() + sent, total - sent);
        if (len == -1) return -1;
        sent += len;
    }
    return sent;
}

void HttpResponse::clear() {
    body_ = "";
    status_code_ = 0;
    version_ = "";
    message_ = "";
    header_.content_length_ = 0;
    header_.content_type_ = "";
    header_.transfer_encoding_ = "";
    location_ = "";
}

HttpResponse HttpResponse::render_default_error_page(int status_code) {
    HttpResponse response;
    std::stringstream ss;
    ss << "<!DOCTYPE html>\n"
       << "<html>\n"
       << "<head>\n"
       << "<title>" << status_code << "</title>\n"
       << "</head>\n"
       << "<body>" << HttpStatus::reason(status_code) << "</body>\n"
       << "</html>\n";
    response.status_code_ = status_code;
    response.body_ = ss.str();
    response.header_.content_type_ = "text/html";
    response.header_.content_length_ = response.body_.size();
    return response;
}

HttpResponse HttpResponse::render_error(
    int status_code, const std::map<int, ErrorPageDirective>& error_pages,
    const ServerConfig& servers) {
    HttpResponse response;
    response.status_code_ = status_code;
    response.message_ = HttpStatus::reason(status_code);
    const std::map<int, ErrorPageDirective>::const_iterator error_page =
        error_pages.find(status_code);
    if (error_page == error_pages.end()) {
        return render_default_error_page(status_code);
    }

    const LocationConfig& location =
        Router::find_location(servers, error_page->second.target);
    int fd;
    for (std::vector<std::string>::const_iterator index_file =
             location.getCommonConfig().index_files_.begin();
         index_file != location.getCommonConfig().index_files_.end();
         ++index_file) {
        fd = open((error_page->second.target +
                   location.getCommonConfig().root_.value_ + *index_file)
                      .c_str(),
                  O_RDONLY);
        if (fd == -1) continue;
        break;
    }
    char buf[1024];  // NOLINT
    std::string buffer;
    while (true) {
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len == -1) throw std::runtime_error("read() failed");
        if (len == 0) break;
        buffer.append(buf);
    }
    response.body_ = buffer;
    return response;
}
