#include <viewshell/application.h>
#include <algorithm>
#include <regex>
#include <set>

namespace viewshell {
namespace detail {

static bool is_valid_origin(const std::string& origin) {
  static const std::regex origin_re(R"(^[a-zA-Z][a-zA-Z0-9+.\-]*://[^:/]+(:\d+)?$)");
  return std::regex_match(origin, origin_re);
}

static std::string normalize_origin(const std::string& origin) {
  auto pos = origin.find("://");
  if (pos == std::string::npos) return origin;
  std::string scheme = origin.substr(0, pos);
  std::string rest = origin.substr(pos + 3);

  std::string default_port;
  if (scheme == "https") default_port = ":443";
  else if (scheme == "http") default_port = ":80";

  auto colon = rest.rfind(':');
  if (colon == std::string::npos && !default_port.empty()) {
    return origin;
  }
  if (colon != std::string::npos) {
    std::string host = rest.substr(0, colon);
    std::string port = rest.substr(colon);
    if (port == default_port) {
      return scheme + "://" + host;
    }
  }
  return origin;
}

Result<NormalizedAppOptions> normalize_app_options_for_test(const AppOptions& options) {
  if (options.bridge_timeout_ms <= 0) {
    return tl::unexpected(Error{"invalid_config",
        "bridge_timeout_ms must be a positive integer"});
  }

  for (const auto& origin : options.trusted_origins) {
    if (!is_valid_origin(origin)) {
      return tl::unexpected(Error{"invalid_config",
          "invalid trusted origin: " + origin});
    }
  }

  std::set<std::string> seen;
  std::vector<std::string> normalized_origins;
  for (const auto& origin : options.trusted_origins) {
    std::string norm = normalize_origin(origin);
    if (seen.insert(norm).second) {
      normalized_origins.push_back(norm);
    }
  }

  return NormalizedAppOptions{
    options.bridge_timeout_ms,
    std::move(normalized_origins),
    options.require_engine,
    options.engine_path
  };
}

} // namespace detail
} // namespace viewshell
