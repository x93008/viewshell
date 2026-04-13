#pragma once

#include <memory>

namespace viewshell {

class BackendRuntime;

class BackendFactory {
public:
  static std::unique_ptr<BackendRuntime> create();
};

} // namespace viewshell
