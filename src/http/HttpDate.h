#pragma once

#include <ctime>
#include <iostream>
#include <string>

class HttpDate {
   public:
    static std::string getCurrentGMT();
    HttpDate();
    const std::time_t& getTime() const { return time_; }

   private:
    std::time_t time_;
    static const int BUFFER_SIZE = 32;
};
