#include "handler_cgi.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../core/Common.h"
#include "../http/HttpParser.h"
#include "request_cgi.h"

namespace {
const char* const LF_LF = "\n\n";
const size_t LF_LF_LENGTH = 2;

const char* const STATUS_HEADER_LOWER = "status";
const char* const CONTENT_TYPE_HEADER = "content-type";
const char* const LOCATION_HEADER_LOWER = "location";

const int MIN_STATUS = 100;
const int MAX_STATUS = 599;
}  // namespace

CgiProcess::CgiProcess() {}

CgiProcess::~CgiProcess() {}

HttpResponse CgiProcess::validateCgiScript(const std::string& script_path) {
    HttpResponse response;
    struct stat st;

    response.status_code_ = HttpStatus::OK;
    if (stat(script_path.c_str(), &st) == -1) {
        response.status_code_ = HttpStatus::NotFound;
        response.body_ = "CGI script not found.";
        return response;
    }

    if (!S_ISREG(st.st_mode)) {
        response.status_code_ = HttpStatus::Forbidden;
        response.body_ = "CGI target is not a regular file.";
        return response;
    }

    if (access(script_path.c_str(), X_OK) != 0 ||
        access(script_path.c_str(), R_OK) != 0) {
        response.status_code_ = HttpStatus::Forbidden;
        response.body_ = "CGI script is not executable or readable";
        return response;
    }

    return response;
}

CgiSession CgiProcess::startCgi(const HttpRequest& request,
                                const RouteInfo& info) {
    char** argv = NULL;
    char** envp = NULL;
    HttpResponse response;
    try {
        std::string path_str = request.request_target_.path_;

        std::string script_path;
        if (info.resolve_.root_.is_set_)
            script_path = info.resolve_.root_.value_ + path_str;

        CgiSession session;
        response = validateCgiScript(script_path);
        if (response.status_code_ != HttpStatus::OK) {
            throw HttpException(response.status_code_);
        }
        argv = createArgv(script_path);
        envp = createEnvp(request);

        CgiResult result = executor_.execute(script_path, argv, envp);

        session.pid = result.pid;
        session.readFd = result.readFd;
        session.writeFd = result.writeFd;
        session.bodyBuffer = request.body_;
        session.sentBytes = 0;
        session.startTime = std::time(NULL);
        deleteArray(argv);
        deleteArray(envp);
        return session;
    } catch (const CgiExecutionException& e) {
        // Handle internal execution errors (500)
        std::cerr << "CGI Execution Error: " << e.what() << "\n";
        deleteArray(argv);
        deleteArray(envp);
        throw HttpException(HttpStatus::InternalServerError);

    } catch (const std::exception& e) {
        // Handle any other standard errors (e.g., std::bad_alloc)
        deleteArray(argv);
        deleteArray(envp);
        throw;  // Re-throw to let ClientHandler handle it
    }
}

void parseCgiHeaderLine(const std::string& line, HttpResponse& response) {
    std::string::size_type colon_pos = line.find(':');

    if (colon_pos == std::string::npos || colon_pos == 0) {
        return;
    }

    std::string header_name = line.substr(0, colon_pos);
    std::string header_value = line.substr(colon_pos + 1);

    header_value = trim(header_value);
    header_name = trim(header_name);
    std::transform(header_name.begin(), header_name.end(), header_name.begin(),
                   ::tolower);

    if (header_name == STATUS_HEADER_LOWER) {
        std::stringstream ss_status(header_value);
        int parsed_status = 0;

        if (ss_status >> parsed_status) {
            if (parsed_status >= MIN_STATUS && parsed_status <= MAX_STATUS) {
                response.status_code_ = parsed_status;
            }
        }
        // else: 変換出来無い場合はデフォルト(200)のまま

    } else if (header_name == CONTENT_TYPE_HEADER) {
        response.header_.content_type_ = header_value;

    } else if (header_name == LOCATION_HEADER_LOWER) {
        response.location_ = header_value;
    }
}

void CgiProcess::parseCgiResponse(HttpResponse& response,
                                  const std::string& raw_output) {
    std::string::size_type pos = raw_output.find(LF_LF);

    std::string header_str;
    std::string::size_type body_start_pos;

    if (pos != std::string::npos) {
        header_str = raw_output.substr(0, pos);
        body_start_pos = pos + LF_LF_LENGTH;
        response.body_ = raw_output.substr(body_start_pos);
    } else {
        // CGIスクリプトがプロトコル違反の応答をした
        response.status_code_ = HttpStatus::InternalServerError;
        response.body_ = "CGI script returned malformed response";
        return;
    }

    std::stringstream ss(header_str);
    std::string line;

    response.status_code_ = HttpStatus::OK;

    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        parseCgiHeaderLine(line, response);
    }
    response.header_.content_length_ = response.body_.size();
    if (!response.location_.empty() &&
        response.status_code_ == HttpStatus::OK) {
        response.status_code_ = HttpStatus::Found;
    }

    response.message_ = HttpStatus::reason(response.status_code_);
    if (response.status_code_ == HttpStatus::NoContent) {
        return;
    }

    if ((response.status_code_ == HttpStatus::OK ||
         response.status_code_ == HttpStatus::Created) &&
        response.location_.empty() &&            // リダイレクトでもない
        response.header_.content_type_.empty())  // Content-Typeが空
    {
        // レスポンスが不正なので 500に上書きする
        response.status_code_ = HttpStatus::InternalServerError;
        response.message_ = HttpStatus::reason(response.status_code_);
        response.body_ =
            "CGI script returned malformed response (missing Content-Type)";
    }
}
