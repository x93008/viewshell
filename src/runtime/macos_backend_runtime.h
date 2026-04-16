#pragma once

#ifdef __APPLE__

#include <memory>

#include "runtime/backend_runtime.h"

namespace viewshell {

class MacOSWindowHost;

class MacOSBackendRuntime final : public BackendRuntime {
public:
  ~MacOSBackendRuntime() override;

  Result<std::shared_ptr<WindowHost>> create_window(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const NormalizedAppOptions& app_options,
      const WindowOptions& window_options) override;

  Result<void> post(std::shared_ptr<RuntimeAppState> app_state,
      std::function<void()> task) override;

  Result<int> run(std::shared_ptr<RuntimeAppState> app_state) override;

private:
  std::vector<std::shared_ptr<MacOSWindowHost>> active_hosts_;
};

} // namespace viewshell

#endif
