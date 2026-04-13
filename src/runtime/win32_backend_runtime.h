#pragma once

#include <memory>

#include "runtime/backend_runtime.h"

namespace viewshell {

class Win32WindowHost;

class Win32BackendRuntime final : public BackendRuntime {
public:
  ~Win32BackendRuntime() override;

  Result<std::shared_ptr<WindowHost>> create_window(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const NormalizedAppOptions& app_options,
      const WindowOptions& window_options) override;

  Result<void> post(std::shared_ptr<RuntimeAppState> app_state,
      std::function<void()> task) override;

  Result<int> run(std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state) override;

private:
  std::shared_ptr<Win32WindowHost> active_host_;
};

} // namespace viewshell
