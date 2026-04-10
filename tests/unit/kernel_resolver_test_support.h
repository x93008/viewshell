#pragma once

#include "platform/linux_x11/kernel_resolver.h"

namespace viewshell {

inline Result<ResolvedEngine> ResolveWithProbeForTest(KernelResolver::ProbeFn probe) {
  AppOptions options;
  return KernelResolver::resolve_with_probe(options, probe);
}

} // namespace viewshell
