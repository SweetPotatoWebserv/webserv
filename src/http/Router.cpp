#include "Router.h"

Router::Router(const HttpConfig& config) : config_(config) {}

const ServerConfig& Router::find_server(const HttpRequest& request) const {
    const ServerConfig* matched = NULL;
    const std::vector<ServerConfig>& servers = config_.getservers();
    for (std::vector<ServerConfig>::const_iterator servers_itr =
             servers.begin();
         servers_itr != servers.end(); ++servers_itr) {
        if (servers_itr->getListens().port != request.host_.getPort()) continue;
        if (servers_itr->getListens().address != DEFAULT_ADDRESS &&
            servers_itr->getListens().address != request.host_.getAddress())
            continue;
        for (std::vector<std::string>::const_iterator server_names =
                 servers_itr->getServerNames().begin();
             server_names != servers_itr->getServerNames().end();
             ++server_names) {
            if (*server_names == request.host_.getAddress()) {
                matched = &(*servers_itr);
                break;
            }
        }
        if (matched) break;
    }
    if (!matched && !servers.empty()) matched = &servers.front();
    if (!matched)
        throw HttpException(HttpStatus::NotFound,
                            HttpStatus::reason(HttpStatus::NotFound));
    return *matched;
}

const LocationConfig& Router::find_location(const ServerConfig& server,
                                            const std::string& path) {
    const LocationConfig* location = NULL;
    for (std::vector<LocationConfig>::const_iterator locations =
             server.getLocations().begin();
         locations != server.getLocations().end(); ++locations) {
        if (path == locations->getPath()) {
            location = &(*locations);
            break;
        }
    }
    if (!location)
        throw HttpException(HttpStatus::NotFound,
                            HttpStatus::reason(HttpStatus::NotFound));
    return *location;
}

RouteInfo Router::route(const HttpRequest& request) const {
    RouteInfo info;

    info.server_ = &find_server(request);
    info.location_ =
        &find_location(*info.server_, request.request_target_.path_);
    info.resolve_ =
        ResolveConfig::resolve_config(config_, *info.server_, *info.location_);
    return info;
}
