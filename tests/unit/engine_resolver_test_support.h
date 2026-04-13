#pragma once

#include "webview/engine_resolver.h"

namespace viewshell {

inline Result<ResolvedEngine> ResolveWithProbeForTest(EngineResolver::ProbeFn probe) {
  AppOptions options;
  return EngineResolver::resolve_with_probe(options, probe);
}

} // namespace viewshell
