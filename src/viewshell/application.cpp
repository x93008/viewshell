#include <viewshell/application.h>
#include "runtime_state.h"

namespace viewshell {

Application::Application(NormalizedAppOptions opts)
    : opts_(std::move(opts)),
      window_state_(std::make_shared<RuntimeWindowState>()) {}

Application::Application(Application&& other) noexcept
    : opts_(std::move(other.opts_)),
      window_state_(std::move(other.window_state_)) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    opts_ = std::move(other.opts_);
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
  if (window_state_->has_window) {
    return tl::unexpected(Error{"multiple_windows_unsupported",
        "only one window is supported in the first release"});
  }
  window_state_->has_window = true;
  return WindowHandle(window_state_);
}

Result<void> Application::post(std::function<void()> task) {
  (void)task;
  return tl::unexpected(Error{"invalid_state",
      "post is not yet available"});
}

Result<int> Application::run() {
  if (!window_state_->has_window) {
    return tl::unexpected(Error{"invalid_state",
        "a window must be created before run"});
  }
  return 0;
}

} // namespace viewshell
