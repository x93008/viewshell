#pragma once

#include <string>
#include <vector>
#include <functional>
#include <viewshell/options.h>
#include <viewshell/capabilities.h>
#include <viewshell/types.h>

namespace viewshell {

struct ResolvedEngine {
  std::string engine_id;
  std::string library_path;
  Capabilities capabilities;
};

class EngineResolver {
public:
  struct ProbeResult {
    bool library_found = false;
    bool init_success = false;
    bool required_probes_ok = false;
    Capabilities capabilities;
  };

  using ProbeFn = std::function<ProbeResult(std::string_view candidate_path)>;

  static Result<ResolvedEngine> resolve(const AppOptions& options);
  static std::vector<std::string> candidate_paths(const AppOptions& options);
  static Result<ResolvedEngine> resolve_with_probe(const AppOptions& options, ProbeFn probe);
};

} // namespace viewshell
