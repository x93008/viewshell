#ifdef _WIN32

#include "win32_backend_runtime.h"

#include <mutex>

#include "host/windows/win32_window_host.h"
#include "viewshell/runtime_state.h"

namespace viewshell {

Win32BackendRuntime::~Win32BackendRuntime() = default;

Result<std::shared_ptr<WindowHost>> Win32BackendRuntime::create_window(
    std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state,
    const NormalizedAppOptions&, const WindowOptions& window_options) {
  auto host = Win32WindowHost::create(app_state, window_state, window_options);
  if (!host) {
    return tl::unexpected(host.error());
  }

  active_hosts_.push_back(*host);
  return std::static_pointer_cast<WindowHost>(*host);
}

Result<void> Win32BackendRuntime::post(std::shared_ptr<RuntimeAppState> app_state,
    std::function<void()> task) {
  if (!app_state->run_started || app_state->shutdown_started) {
    return tl::unexpected(Error{"invalid_state",
        "post is only valid after run() starts and before shutdown"});
  }

  {
    std::lock_guard<std::mutex> lock(app_state->mutex);
    app_state->posted_tasks.push_back(std::move(task));
  }
  app_state->cv.notify_one();
  return {};
}

Result<int> Win32BackendRuntime::run(std::shared_ptr<RuntimeAppState> app_state) {
  if (app_state->windows.empty() || active_hosts_.empty()) {
    return tl::unexpected(Error{"invalid_state",
        "a window must be created before run"});
  }
  if (app_state->shutdown_started) {
    return app_state->run_exit_code;
  }

  app_state->run_started = true;
  active_hosts_.front()->run_message_loop();
  return app_state->run_exit_code;
}

} // namespace viewshell

#endif
