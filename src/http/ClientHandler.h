#pragma once
#include "HttpParser.h"

class ClientHandler {
   public:
    void onReadable();
    void onWritable();

   private:
    HttpParser parser_;
    HttpRequest request_;
    HttpResponse response_;
};
