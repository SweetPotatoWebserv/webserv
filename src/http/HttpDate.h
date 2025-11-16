#pragma once

#include <ctime>
#include <iostream>
#include <string>

class HttpDate {
   public:
    static std::string getCurrentGMT();

   private:
    std::time_t timestamp_;
    static const int BUFFER_SIZE = 32;
};
