#include "Router.h"

Router::Router(const HttpConfig& config) : config_(config) {}

const LocationConfig& Router::find_location(const ServerConfig& server,
                                            const std::string& path) {
    const std::vector<LocationConfig>& locations = server.getLocations();
    const LocationConfig* location = NULL;
    size_t match_len = 0;

    for (std::vector<LocationConfig>::const_iterator locations_itr =
             locations.begin();
         locations_itr != locations.end(); ++locations_itr) {
        const std::string& location_path = locations_itr->getPath();

        if (path.find(location_path) == 0) {
            if (location_path.size() > match_len) {
                location = &(*locations_itr);
                match_len = location_path.size();
            }
        }
    }
    if (!location)
        throw HttpException(HttpStatus::NotFound,
                            HttpStatus::reason(HttpStatus::NotFound));
    return *location;
}

RouteInfo Router::route(const ServerConfig& server_config,
                        const HttpRequest& request) const {
    RouteInfo info;

    info.server_ = &server_config;
    info.location_ =
        &find_location(*info.server_, request.request_target_.path_);
    info.resolve_ =
        ResolveConfig::resolve_config(config_, *info.server_, *info.location_);
    return info;
}
