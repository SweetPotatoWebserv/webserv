#include "MimeTypes.h"

#include <algorithm>

std::string MimeTypes::extract_extension(const std::string& file_name) {
    std::string::size_type pos = file_name.rfind('.');
    if (pos == std::string::npos) return "";
    // . を飛ばすため、+1
    return file_name.substr(pos + 1);
}

std::map<std::string, std::string> MimeTypes::create_content_type_map() {
    std::map<std::string, std::string> content_type_map;
    content_type_map["html"] = "text/html; charset=UTF-8";
    content_type_map["htm"] = "text/html; charset=UTF-8";
    content_type_map["css"] = "text/css; charset=UTF-8";
    content_type_map["js"] = "application/javascript; charset=UTF-8";
    content_type_map["json"] = "application/json; charset=UTF-8";
    content_type_map["txt"] = "text/plain; charset=UTF-8";
    content_type_map["jpg"] = "image/jpeg";
    content_type_map["jpeg"] = "image/jpeg";
    content_type_map["png"] = "image/png";
    content_type_map["gif"] = "image/gif";
    content_type_map["svg"] = "image/svg+xml";
    content_type_map["ico"] = "image/x-icon";
    content_type_map["pdf"] = "application/pdf";
    content_type_map["xml"] = "application/xml";
    content_type_map["zip"] = "application/zip";
    content_type_map["gz"] = "application/gzip";
    content_type_map["tar"] = "application/x-tar";
    content_type_map["mp3"] = "audio/mpeg";
    content_type_map["mp4"] = "video/mp4";
    content_type_map["avi"] = "video/x-msvideo";
    content_type_map["php"] = "application/x-httpd-php";
    content_type_map["py"] = "text/x-python";
    content_type_map["c"] = "text/x-c";
    content_type_map["cpp"] = "text/x-c++";
    return content_type_map;
}

std::string MimeTypes::get_mime_type(const std::string& file_name) {
    std::string extension = extract_extension(file_name);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   ::tolower);
    if (extension.empty()) return "application/octet-stream";
    static const std::map<std::string, std::string> ContentTypeMap =
        create_content_type_map();
    std::map<std::string, std::string>::const_iterator found =
        ContentTypeMap.find(extension);
    if (found != ContentTypeMap.end()) {
        return found->second;
    }
    return "application/octet-stream";
}
