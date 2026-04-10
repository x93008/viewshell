#include "kernel_resolver.h"

#include <algorithm>

namespace viewshell {

namespace {

bool is_compatible_engine(const std::string& engine) {
  return engine == "webkit" || engine == "webkitgtk";
}

} // namespace

std::vector<std::string> KernelResolver::candidate_paths(const AppOptions& options) {
  std::vector<std::string> paths;
  if (options.engine_path.has_value()) {
    paths.push_back(*options.engine_path);
  }
  paths.push_back("libwebkit2gtk-4.1.so.0");
  paths.push_back("libwebkit2gtk-4.0.so.37");
  return paths;
}

Result<ResolvedEngine> KernelResolver::resolve_with_probe(const AppOptions& options, ProbeFn probe) {
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

Result<ResolvedEngine> KernelResolver::resolve(const AppOptions& options) {
  ProbeFn default_probe = [](std::string_view candidate) -> ProbeResult {
    return {.library_found = false, .init_success = false, .required_probes_ok = false};
  };
  return resolve_with_probe(options, default_probe);
}

} // namespace viewshell
