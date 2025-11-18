#pragma once

#include <string>

#include "../http/HttpParser.h"

std::string extractQueryString(const std::string& uri);
char** createArgv(const std::string& script_path);
char** createEnvp(const HttpRequest& request);
void freeArray(char** arr);
