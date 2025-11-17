#include "ResponseFactory.h"
#include "HttpParser.h"
#include "Router.h"
#include "MimeTypes.h"

HttpResponse ResponseFactory::response_get(const RouteInfo& route) {
    HttpResponse response;
    std::string path_name;
    bool found = false;
    for (std::vector<std::string>::const_iterator index_files = route.resolve_.index_files_.begin();
         index_files != route.resolve_.index_files_.end(); ++index_files) {
        path_name =  route.resolve_.root_.value_ + route.location_->getPath() + *index_files;
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
    //TODO 404 エラーを返す
    if (!found) {
        std::cout << "404\n";
        // render_error_page()
    }
    response.status_code_ = HttpStatus::OK;
    response.message_ = HttpStatus::reason(HttpStatus::OK);
    response.header_.content_length_ = response.body_.size();
    response.header_.content_type_ = MimeTypes::get_mime_type(path_name);
    return response;
}

// HttpResponse ResponseFactory::response_post() {
//
// }
// HttpResponse ResponseFactory::response_delete() {
//
// }

HttpResponse ResponseFactory::make(const HttpRequest& request, const RouteInfo& route, const HttpException& parse_error) {
    (void)parse_error;
    // パースエラー
    // if (parse_error.status_code() != HttpStatus::OK) {
    //     return HttpResponse::render_error(parse_error.status_code(), route);
    // }
    // TODO allowedメソッドがまだ実装されてないため、コメントアウト
    // if (std::find(resolve_.allowed_methods_.begin(),
    //               resolve_.allowed_methods_.end(),
    //               request.method_) == resolve_.allowed_methods_.end())
    //                                       resolve_.error_page_, server);
    //     return HttpResponse::render_error(HttpStatus::MethodNotAllowed,
    switch (request.method_) {
        case MethodGET:
            return response_get(route);
        // case MethodHEAD:
        //     HttpResponse response = response_get();
        //     // responseからbodyを削除
        //     return response;
        // case MethodPOST:
        //     return response_post();
        // case MethodDELETE:
        //     return response_delete();
        default:
            throw std::runtime_error("パースの時点で例外を投げてるため、入らないはず。入った時はなんかおかしいから例外を投げて気づけるようにする");
            break;
    }
}
