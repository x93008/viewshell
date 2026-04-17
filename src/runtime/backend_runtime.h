#pragma once

#include <functional>
#include <memory>

#include <viewshell/options.h>
#include <viewshell/tray_options.h>
#include <viewshell/types.h>

namespace viewshell {

struct RuntimeAppState;
struct RuntimeWindowState;
class WindowHost;
class TrayHost;

class BackendRuntime {
public:
  virtual ~BackendRuntime() = default;

  virtual Result<std::shared_ptr<WindowHost>> create_window(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const NormalizedAppOptions& app_options,
      const WindowOptions& window_options) = 0;

  virtual Result<std::shared_ptr<TrayHost>> create_tray(
      const TrayOptions& options) = 0;

  virtual Result<void> post(std::shared_ptr<RuntimeAppState> app_state,
      std::function<void()> task) = 0;

  virtual Result<int> run(std::shared_ptr<RuntimeAppState> app_state) = 0;
};

} // namespace viewshell
