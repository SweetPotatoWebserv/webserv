#include "HttpResponse.h"

#include <fcntl.h>

#include "MimeTypes.h"
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

HttpResponse HttpResponse::render_error(int status_code,
                                        const RouteInfo& route) {
    HttpResponse response;
    int out_status = status_code;
    if (route.resolve_.error_page_.empty())
        return render_default_error_page(status_code);

    std::map<int, ErrorPageDirective>::const_iterator target_error_page =
        route.resolve_.error_page_.find(status_code);
    if (target_error_page == route.resolve_.error_page_.end())
        return render_default_error_page(status_code);

    const ErrorPageDirective& error_page_directive = target_error_page->second;
    if (error_page_directive.override_status != CommonConfig::INVALID_NUM) {
        out_status = error_page_directive.override_status;
    }

    int fd = open(error_page_directive.target.c_str(), O_RDONLY);
    if (fd == -1) return render_default_error_page(status_code);

    std::vector<char> buffer;
    char buf[DEFAULT_BUFFER_SIZE];
    while (true) {
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len == -1) {
            close(fd);
            throw std::runtime_error("read() failed");
        }
        if (len == 0) break;
        buffer.insert(buffer.end(), buf, buf + len);
    }
    close(fd);

    response.status_code_ = out_status;
    response.message_ = HttpStatus::reason(out_status);
    response.body_.assign(buffer.begin(), buffer.end());
    response.header_.content_length_ = response.body_.size();
    response.header_.content_type_ =
        MimeTypes::get_mime_type(error_page_directive.target);
    return response;
}
