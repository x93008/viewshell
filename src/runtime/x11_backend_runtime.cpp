#include "x11_backend_runtime.h"

#include <mutex>

#include "webview/engine_resolver.h"
#include "host/x11/x11_window_host.h"
#include "viewshell/runtime_state.h"

namespace viewshell {
namespace {

AppOptions to_app_options(const NormalizedAppOptions& options) {
  AppOptions app_options;
  app_options.bridge_timeout_ms = options.bridge_timeout_ms;
  app_options.trusted_origins = options.trusted_origins;
  app_options.require_engine = options.require_engine;
  app_options.engine_path = options.engine_path;
  return app_options;
}

} // namespace

X11BackendRuntime::~X11BackendRuntime() = default;

Result<std::shared_ptr<WindowHost>> X11BackendRuntime::create_window(
    std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state,
    const NormalizedAppOptions& app_options,
    const WindowOptions& window_options) {
  auto resolved = EngineResolver::resolve(to_app_options(app_options));
  if (!resolved) {
    return tl::unexpected(resolved.error());
  }

  auto host = X11WindowHost::create(app_state, window_state, window_options);
  if (!host) {
    return tl::unexpected(host.error());
  }

  active_hosts_.push_back(*host);
  return std::static_pointer_cast<WindowHost>(*host);
}

Result<void> X11BackendRuntime::post(std::shared_ptr<RuntimeAppState> app_state,
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

Result<int> X11BackendRuntime::run(std::shared_ptr<RuntimeAppState> app_state) {
  if (app_state->windows.empty() || active_hosts_.empty()) {
    return tl::unexpected(Error{"invalid_state",
        "a window must be created before run"});
  }
  if (app_state->shutdown_started) {
    return app_state->run_exit_code;
  }

  app_state->run_started = true;
  active_hosts_.front()->run_main_loop();
  return app_state->run_exit_code;
}

} // namespace viewshell
