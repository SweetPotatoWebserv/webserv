#include "ResponseFactory.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <sstream>

#include "HttpParser.h"
#include "MimeTypes.h"
#include "Router.h"

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
        ssize_t len;
        while ((len = read(fd, buf, DEFAULT_BUFFER_LEN)) > 0) {
            buffer.insert(buffer.end(), buf, buf + len);
        }
        close(fd);
        response.body_.assign(buffer.begin(), buffer.end());
        found = true;
        break;
    }
    if (!found) {
        return HttpResponse::render_error(HttpStatus::NotFound, route);
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
        return HttpResponse::render_error(HttpStatus::InternalServerError,
                                          route);
    const std::string& store_dir = route.resolve_.upload_store_.value_;
    std::string dir = store_dir;
    if (dir[dir.size() - 1] != '/') dir += "/";

    std::stringstream ss;
    ss << "upload_" << time(NULL);
    std::string filename = ss.str();
    std::string fullpath = dir + filename;
    int fd =
        open(fullpath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);  // NOLINT
    if (fd == -1)
        return HttpResponse::render_error(HttpStatus::InternalServerError,
                                          route);

    const std::string& body = request.body_;
    size_t total = body.size();
    ssize_t sent = 0;

    while (sent < static_cast<ssize_t>(total)) {
        ssize_t len = write(fd, body.c_str() + sent, total - sent);
        if (len == -1)
            return HttpResponse::render_error(HttpStatus::InternalServerError,
                                              route);
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

HttpResponse ResponseFactory::response_delete(const HttpRequest& request,
                                              const RouteInfo& route) {
    if (!route.resolve_.root_.is_set_)
        return HttpResponse::render_error(HttpStatus::InternalServerError,
                                          route);
    std::string path =
        route.resolve_.root_.value_ + request.request_target_.path_;
    struct stat st;
    if (stat(path.c_str(), &st) == -1)
        return HttpResponse::render_error(HttpStatus::NotFound, route);

    if (S_ISDIR(st.st_mode))
        return HttpResponse::render_error(HttpStatus::Forbidden, route);
    if (unlink(path.c_str()) == -1) {
        if (errno == EACCES) {
            return HttpResponse::render_error(HttpStatus::Forbidden, route);
        }
        return HttpResponse::render_error(HttpStatus::InternalServerError,
                                          route);
    }
    HttpResponse response;
    response.status_code_ = HttpStatus::NoContent;
    response.message_ = HttpStatus::reason(HttpStatus::NoContent);
    response.header_.content_length_ = 0;
    return response;
}

HttpResponse ResponseFactory::make(const HttpRequest& request,
                                   const RouteInfo& route,
                                   const HttpException& parse_error) {
    // パースエラー
    if (parse_error.status_code() != HttpStatus::OK) {
        return HttpResponse::render_error(parse_error.status_code(), route);
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
            throw std::runtime_error(
                "パースの時点で例外を投げてるため、入らないはず。入った時はなん"
                "かおかしいから例外を投げて気づけるようにする");
            break;
    }
}
