#pragma once

#include <memory>

#include "runtime/backend_runtime.h"

namespace viewshell {

class X11WindowHost;

class X11BackendRuntime final : public BackendRuntime {
public:
  ~X11BackendRuntime() override;

  Result<std::shared_ptr<WindowHost>> create_window(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const NormalizedAppOptions& app_options,
      const WindowOptions& window_options) override;

  Result<std::shared_ptr<TrayHost>> create_tray(
      const TrayOptions& options) override;

  Result<void> post(std::shared_ptr<RuntimeAppState> app_state,
      std::function<void()> task) override;

  Result<int> run(std::shared_ptr<RuntimeAppState> app_state) override;

private:
  std::vector<std::shared_ptr<X11WindowHost>> active_hosts_;
};

} // namespace viewshell
