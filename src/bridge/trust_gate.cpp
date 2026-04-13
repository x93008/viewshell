#include "bridge/trust_gate.h"
#include <algorithm>

namespace viewshell {

namespace {

std::string extract_origin(const std::string& url) {
  auto scheme_end = url.find("://");
  if (scheme_end == std::string::npos) return url;
  auto after_scheme = url.substr(scheme_end + 3);
  auto path_start = after_scheme.find('/');
  if (path_start == std::string::npos) return url.substr(0, scheme_end + 3 + after_scheme.size());
  return url.substr(0, scheme_end + 3 + path_start);
}

} // namespace

TrustDecision TrustGate::classify(
    const std::string& url,
    const std::vector<std::string>& trusted_origins) {
  if (is_local_app(url)) {
    Capabilities caps;
    caps.window.borderless = true;
    caps.window.transparent = true;
    caps.window.always_on_top = true;
    caps.window.native_drag = true;
    caps.webview.devtools = true;
    caps.webview.resource_protocol = true;
    caps.webview.script_eval = true;
    caps.bridge.invoke = true;
    caps.bridge.native_events = true;
    return {TrustMode::FullBridge, caps};
  }

  auto origin = extract_origin(url);
  bool trusted = std::find(trusted_origins.begin(), trusted_origins.end(), origin)
                 != trusted_origins.end();

  if (trusted) {
    Capabilities caps;
    caps.window.borderless = true;
    caps.window.always_on_top = true;
    caps.bridge.invoke = true;
    caps.bridge.native_events = true;
    return {TrustMode::ReducedBridge, caps};
  }

  return {TrustMode::NoBridge, Capabilities{}};
}

bool TrustGate::is_local_app(const std::string& url) {
  return url.rfind("viewshell://app/", 0) == 0;
}

} // namespace viewshell
