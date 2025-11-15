#pragma once

#include <string>

#include "../http/HttpParser.h"

char** createArgv(const std::string& script_path);
char** createEnvp(const HttpRequest& request);
void freeArray(char** arr);
