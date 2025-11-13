#pragma once

#include <map>

#include <string>

class MimeTypes {
   public:
    static std::string get_mime_type(const std::string& file_name);

   private:
    static std::string extract_extension(const std::string& file_name);
    static std::map<std::string, std::string> create_content_type_map();
};
