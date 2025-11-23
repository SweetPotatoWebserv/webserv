#include <sys/types.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <string>

#include "../core/Common.h"
#include "../http/HttpParser.h"

template <typename T>
std::string numToString(T number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string methodToString(Method method) {
    if (method == MethodGET) return "GET";
    if (method == MethodPOST) return "POST";
    if (method == MethodDELETE) return "DELETE";
    if (method == MethodHEAD) return "HEAD";
    if (method == MethodUNKNOWN) return "UNKNOWN";
    return "";
}

char** createArgv(const std::string& script_path) {
    char** argv = new char*[2];
    argv[0] = strdup(script_path.c_str());
    argv[1] = NULL;
    if (argv[0] == NULL) {
        delete[] argv;
        return NULL;
    }
    return argv;
}

char** createEnvp(const HttpRequest& request) {
    std::map<std::string, std::string> env_map;

    env_map["REQUEST_METHOD"] = methodToString(request.method_);
    env_map["CONTENT_LENGTH"] = numToString(request.header_.content_length_);
    env_map["CONTENT_TYPE"] = request.header_.content_type_;
    env_map["SCRIPT_NAME"] = request.request_target_.path_;
    env_map["QUERY_STRING"] = request.request_target_.query_string_;

    env_map["SERVER_NAME"] = request.host_.getAddress();
    env_map["SERVER_PORT"] = numToString(request.host_.getPort());

    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["SERVER_SOFTWARE"] = "webserv/1.0";
    env_map["GATEWAY_INTERFACE"] = "CGI/1.1";

    char** envp = new char*[env_map.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it =
             env_map.begin();
         it != env_map.end(); ++it) {
        std::string env_line = it->first + "=" + it->second;
        envp[i] = new char[env_line.length() + 1];
        if (envp[i] == NULL) {
            for (int j = 0; j < i; ++j) {
                delete (envp[j]);
            }
            delete[] envp;
            return NULL;
        }
        i++;
    }
    envp[i] = NULL;
    return envp;
}

void deleteArray(char** arr) {
    if (!arr) return;
    for (int i = 0; arr[i] != NULL; ++i) {
        delete (arr[i]);
    }
    delete[] arr;
}
