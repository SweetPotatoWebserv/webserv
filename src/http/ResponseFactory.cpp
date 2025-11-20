#include "ResponseFactory.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sstream>
#include <stdexcept>

#include "HttpParser.h"
#include "MimeTypes.h"
#include "Router.h"

HttpResponse ResponseFactory::render_default_error_page(int status_code) {
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

HttpResponse ResponseFactory::render_error(int status_code,
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
    char buf[DEFAULT_BUFFER_LEN];
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

HttpResponse ResponseFactory::response_get(const HttpRequest& request,
                                           const RouteInfo& route) {
    HttpResponse response;
    std::string path_name;
    bool found = false;
    for (std::vector<std::string>::const_iterator index_files =
             route.resolve_.index_files_.begin();
         index_files != route.resolve_.index_files_.end(); ++index_files) {
        path_name = route.resolve_.root_.value_ +
                    request.request_target_.path_ + *index_files;
        int fd = open(path_name.c_str(), O_RDONLY);
        if (fd == -1) {
            continue;
        }
        // std::stringだと画像データなどバイナリに対応できないため、vectorを使用する
        std::vector<char> buffer;
        char buf[DEFAULT_BUFFER_LEN];
        while (true) {
            ssize_t len = read(fd, buf, DEFAULT_BUFFER_LEN);
            if (len == -1) {
                throw std::runtime_error("failed to read: " +
                                         std::string(std::strerror(errno)));
                close(fd);
            }
            if (len == 0) break;
            buffer.insert(buffer.end(), buf, buf + len);
        }
        close(fd);
        response.body_.assign(buffer.begin(), buffer.end());
        found = true;
        break;
    }
    if (!found) {
        std::string file_path =
            route.resolve_.root_.value_ + request.request_target_.path_;
        struct stat status;
        if (stat(file_path.c_str(), &status) == 0 && S_ISDIR(status.st_mode)) {
            if (route.resolve_.autoindex_.is_set_ &&
                route.resolve_.autoindex_.value_) {
                return response_autoindex(request, route);
            }
            return render_error(HttpStatus::Forbidden, route);
        }
        return render_error(HttpStatus::NotFound, route);
    }
    response.status_code_ = HttpStatus::OK;
    response.message_ = HttpStatus::reason(HttpStatus::OK);
    response.header_.content_length_ = response.body_.size();
    response.header_.content_type_ = MimeTypes::get_mime_type(path_name);
    return response;
}

HttpResponse ResponseFactory::response_post(const HttpRequest& request,
                                            const RouteInfo& route) {
    if (!route.resolve_.upload_store_.is_set_)
        return render_error(HttpStatus::InternalServerError, route);
    const std::string& store_dir = route.resolve_.upload_store_.value_;
    std::string dir = store_dir;
    if (dir[dir.size() - 1] != '/') dir += "/";

    std::stringstream ss;
    ss << "upload_" << time(NULL);
    std::string filename = ss.str();
    std::string fullpath = dir + filename;
    int fd =
        open(fullpath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);  // NOLINT
    if (fd == -1) return render_error(HttpStatus::InternalServerError, route);

    const std::string& body = request.body_;
    size_t total = body.size();
    ssize_t sent = 0;

    while (sent < static_cast<ssize_t>(total)) {
        ssize_t len = write(fd, body.c_str() + sent, total - sent);
        if (len == -1)
            return render_error(HttpStatus::InternalServerError, route);
        sent += len;
    }
    close(fd);
    HttpResponse response;
    response.status_code_ = HttpStatus::Created;
    response.message_ = HttpStatus::reason(HttpStatus::Created);
    response.header_.content_type_ = "text/plain";
    std::string message = "Uploaded to " + filename + "\n";
    response.body_ = message;
    response.header_.content_length_ = message.size();
    return response;
}

HttpResponse ResponseFactory::response_autoindex(const HttpRequest& request,
                                                 const RouteInfo& route) {
    HttpResponse response;

    std::string file_path =
        route.resolve_.root_.value_ + request.request_target_.path_;

    DIR* directory = opendir(file_path.c_str());
    if (directory == NULL) {
        return render_error(HttpStatus::InternalServerError, route);
    }

    std::string request_path = request.request_target_.path_;
    if (!request_path.empty() && request_path[request_path.size() - 1] != '/')
        request_path += "/";

    std::vector<std::string> entries;
    while (true) {
        errno = 0;
        struct dirent* directory_entry = readdir(directory);
        if (directory_entry == NULL) break;
        std::string name = directory_entry->d_name;
        if (name == ".") continue;
        if (name == "..") continue;
        entries.push_back(name);
    }
    int saved_errno = errno;
    closedir(directory);
    if (saved_errno != 0) {
        return render_error(HttpStatus::InternalServerError, route);
    }

    std::sort(entries.begin(), entries.end());

    std::ostringstream body;
    body << "<!DOCTYPE html>\n"
         << "<html>\n<head>\n<title>Index of " << request_path
         << "</title>\n</head>\n<body>\n";
    body << "<h1>Index of " << request_path << "</h1>\n<hr><pre>\n";
    if (request_path != "/") {
        body << "<a href=\"../\">../</a>\n";
    }

    for (std::vector<std::string>::iterator entry = entries.begin();
         entry != entries.end(); ++entry) {
        std::string fs_path = file_path + *entry;
        struct stat status;
        std::string display = *entry;
        std::string href = request_path + *entry;
        if (stat(fs_path.c_str(), &status) == 0 && S_ISDIR(status.st_mode)) {
            display += "/";
            href += "/";
        }
        body << "<a href=\"" << href << "\">" << display << "</a>\n";
    }

    body << "</pre><hr>\n";
    body << "</body>\n</html>\n";

    response.status_code_ = HttpStatus::OK;
    response.message_ = HttpStatus::reason(HttpStatus::OK);
    response.header_.content_type_ = "text/html";
    response.body_ = body.str();
    response.header_.content_length_ = response.body_.size();
    return response;
}

HttpResponse ResponseFactory::response_delete(const HttpRequest& request,
                                              const RouteInfo& route) {
    if (!route.resolve_.root_.is_set_)
        return render_error(HttpStatus::InternalServerError, route);
    std::string path =
        route.resolve_.root_.value_ + request.request_target_.path_;
    struct stat st;
    if (stat(path.c_str(), &st) == -1)
        return render_error(HttpStatus::NotFound, route);

    if (S_ISDIR(st.st_mode)) return render_error(HttpStatus::Forbidden, route);
    if (unlink(path.c_str()) == -1) {
        if (errno == EACCES) {
            return render_error(HttpStatus::Forbidden, route);
        }
        return render_error(HttpStatus::InternalServerError, route);
    }
    HttpResponse response;
    response.status_code_ = HttpStatus::NoContent;
    response.message_ = HttpStatus::reason(HttpStatus::NoContent);
    response.header_.content_length_ = 0;
    return response;
}

HttpResponse ResponseFactory::response_redirect(const RouteInfo& route) {
    HttpResponse response;
    response.status_code_ = route.resolve_.redirect_.status;
    response.message_ = HttpStatus::reason(response.status_code_);
    // TODO Config で定義してる定数を Common に移動したらここも置き換える
    if (response.status_code_ >= 302 &&  // NOLINT
        response.status_code_ <= 308) {  // NOLINT
        response.location_ = route.resolve_.redirect_.target;
        response.header_.content_type_ = "text/html";
        std::stringstream oss;
        oss << "<html>\n"
            << "<head>"
            << "<title>" << response.status_code_ << " "
            << HttpStatus::reason(response.status_code_) << "</title>"
            << "</head>\n"
            << "<body>\n"
            << "<center><h1>" << response.status_code_ << " "
            << HttpStatus::reason(response.status_code_) << "</h1></center>\n"
            << "<hr><center>webserv</center>\n"
            << "</body>"
            << "</html>\n";
        response.body_ = oss.str();
        response.header_.content_length_ = response.body_.size();
        return response;
    }

    response.header_.content_type_ = "application/octet-stream";
    response.body_ = route.resolve_.redirect_.text;
    response.header_.content_length_ = response.body_.size();
    return response;
}

HttpResponse ResponseFactory::make(const HttpRequest& request,
                                   const RouteInfo& route,
                                   const HttpException& parse_error) {
    // パースエラー
    if (parse_error.status_code() != HttpStatus::OK) {
        return render_error(parse_error.status_code(), route);
    }
    // TODO allowedメソッドがまだ実装されてないため、コメントアウト
    // if (std::find(resolve_.allowed_methods_.begin(),
    //               resolve_.allowed_methods_.end(),
    //               request.method_) == resolve_.allowed_methods_.end())
    //                                       resolve_.error_page_, server);
    //     return HttpResponse::render_error(HttpStatus::MethodNotAllowed,
    // TODO cgi が追加されたら追加する
    // if (is_cgi(request, route))
    //     return response cgi;
    if (route.resolve_.redirect_.status != CommonConfig::INVALID_NUM)
        return response_redirect(route);

    if (route.resolve_.client_max_body_size_ != CommonConfig::INVALID_NUM &&
        route.resolve_.client_max_body_size_ <
            static_cast<off_t>(request.body_.size()))
        return render_error(HttpStatus::PayloadTooLarge, route);

    switch (request.method_) {
        case MethodGET: {
            return response_get(request, route);
        }
        case MethodHEAD: {
            HttpResponse response = response_get(request, route);
            response.body_.clear();
            return response;
        }
        case MethodPOST:
            return response_post(request, route);
        case MethodDELETE:
            return response_delete(request, route);
        default:
            throw std::runtime_error("Unsupported HTTP method");
            break;
    }
}
