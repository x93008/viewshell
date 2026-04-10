#include <viewshell/application.h>
#include "runtime_state.h"

namespace viewshell {

Application::Application(NormalizedAppOptions opts)
    : opts_(std::move(opts)),
      app_state_(std::make_shared<RuntimeAppState>()),
      window_state_(std::make_shared<RuntimeWindowState>()) {}

Application::Application(Application&& other) noexcept
    : opts_(std::move(other.opts_)),
      app_state_(std::move(other.app_state_)),
      window_state_(std::move(other.window_state_)) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    opts_ = std::move(other.opts_);
    app_state_ = std::move(other.app_state_);
    window_state_ = std::move(other.window_state_);
  }
  return *this;
}

Application::~Application() = default;

Result<Application> Application::create(const AppOptions& options) {
  auto normalized = detail::normalize_app_options_for_test(options);
  if (!normalized) {
    return tl::unexpected(normalized.error());
  }
  return Application(std::move(*normalized));
}

Result<WindowHandle> Application::create_window(const WindowOptions&) {
  if (app_state_->run_started && window_state_->has_window) {
    return tl::unexpected(Error{"multiple_windows_unsupported",
        "only one window is supported in the first release"});
  }
  if (window_state_->has_window) {
    return tl::unexpected(Error{"multiple_windows_unsupported",
        "only one window is supported in the first release"});
  }
  window_state_->has_window = true;
  return WindowHandle(window_state_);
}

Result<void> Application::post(std::function<void()> task) {
  if (!app_state_->run_started || app_state_->shutdown_started) {
    return tl::unexpected(Error{"invalid_state",
        "post is only valid after run() starts and before shutdown"});
  }
  {
    std::lock_guard<std::mutex> lock(app_state_->mutex);
    app_state_->posted_tasks.push_back(std::move(task));
  }
  app_state_->cv.notify_one();
  return {};
}

Result<int> Application::run() {
  if (!window_state_->has_window) {
    return tl::unexpected(Error{"invalid_state",
        "a window must be created before run"});
  }
  if (app_state_->shutdown_started) {
    return app_state_->run_exit_code;
  }
  return 0;
}

} // namespace viewshell
