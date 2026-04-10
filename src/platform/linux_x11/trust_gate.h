#pragma once

#include <string>
#include <vector>
#include <viewshell/capabilities.h>

namespace viewshell {

enum class TrustMode { NoBridge, ReducedBridge, FullBridge };

struct TrustDecision {
  TrustMode mode;
  Capabilities effective_capabilities;
};

class TrustGate {
public:
  static TrustDecision classify(
      const std::string& url,
      const std::vector<std::string>& trusted_origins);

  static bool is_local_app(const std::string& url);
};

} // namespace viewshell
