#pragma once

#include <string>

#include "../core/Common.h"
#include "../http/HttpParser.h"

void parseCgiResponse(HttpResponse& response, const std::string& raw_output);
