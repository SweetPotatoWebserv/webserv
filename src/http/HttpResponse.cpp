#include "HttpResponse.h"

#include "Router.h"

ssize_t HttpResponse::send_response(int client_fd, HttpResponse& response) {
    std::ostringstream oss;

    oss << HTTP_VERSION << " " << response.status_code_ << " "
        << response.message_ << "\r\n";
    // if (response.date_.to_string().size()) // NOLINT
    //     oss << "Date: " << response.date_.to_string() << "\r\n";
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
    // date clear
    // date_ = "";
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
    response.body_ = ss.str();
    response.header_.content_type_ = "text/html";
    response.header_.content_length_ = response.body_.size();
    return response;
}
