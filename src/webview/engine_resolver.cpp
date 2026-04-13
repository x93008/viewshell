#include "webview/engine_resolver.h"

#include <algorithm>
#include <dlfcn.h>

namespace viewshell {

namespace {

bool is_compatible_engine(const std::string& engine) {
  return engine == "webkit" || engine == "webkitgtk";
}

Capabilities linux_webkit_capabilities() {
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
  return caps;
}

} // namespace

std::vector<std::string> EngineResolver::candidate_paths(const AppOptions& options) {
  std::vector<std::string> paths;
  if (options.engine_path.has_value()) {
    paths.push_back(*options.engine_path);
  }
  paths.push_back("libwebkit2gtk-4.1.so.0");
  paths.push_back("libwebkit2gtk-4.0.so.37");
  return paths;
}

Result<ResolvedEngine> EngineResolver::resolve_with_probe(const AppOptions& options, ProbeFn probe) {
  if (options.require_engine.has_value() && !is_compatible_engine(*options.require_engine)) {
    return tl::unexpected(Error{
      .code = "engine_incompatible",
      .message = "Required engine '" + *options.require_engine + "' is not compatible with this platform"
    });
  }

  auto candidates = candidate_paths(options);
  std::vector<std::string> attempted;
  std::vector<std::string> failure_reasons;

  for (const auto& candidate : candidates) {
    attempted.push_back(candidate);
    auto result = probe(candidate);

    if (!result.library_found) {
      failure_reasons.push_back(candidate + ": not found");
      continue;
    }

    if (!result.init_success) {
      return tl::unexpected(Error{
        .code = "engine_init_failed",
        .message = "Engine initialization failed for " + candidate,
        .details = Json{{"candidate", candidate}}
      });
    }

    if (!result.required_probes_ok) {
      failure_reasons.push_back(candidate + ": required probes failed");
      continue;
    }

    return ResolvedEngine{
      .engine_id = "webkitgtk",
      .library_path = candidate,
      .capabilities = result.capabilities
    };
  }

  return tl::unexpected(Error{
    .code = "engine_not_found",
    .message = "No compatible engine found",
    .details = Json{
      {"attempted_candidates", attempted},
      {"failure_reasons", failure_reasons}
    }
  });
}

Result<ResolvedEngine> EngineResolver::resolve(const AppOptions& options) {
  ProbeFn default_probe = [](std::string_view candidate) -> ProbeResult {
    void* handle = dlopen(std::string(candidate).c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
      return {.library_found = false, .init_success = false, .required_probes_ok = false};
    }
    dlclose(handle);
    return {
      .library_found = true,
      .init_success = true,
      .required_probes_ok = true,
      .capabilities = linux_webkit_capabilities(),
    };
  };
  return resolve_with_probe(options, default_probe);
}

} // namespace viewshell
