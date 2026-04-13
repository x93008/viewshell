#include <viewshell/application.h>
#include "runtime_state.h"
#include "runtime/backend_factory.h"
#include "runtime/backend_runtime.h"

namespace viewshell {

Application::Application(NormalizedAppOptions opts)
    : opts_(std::move(opts)),
      app_state_(std::make_shared<RuntimeAppState>()),
      window_state_(std::make_shared<RuntimeWindowState>()),
      backend_runtime_(BackendFactory::create()) {}

Application::Application(Application&& other) noexcept
    : opts_(std::move(other.opts_)),
      app_state_(std::move(other.app_state_)),
      window_state_(std::move(other.window_state_)),
      backend_runtime_(std::move(other.backend_runtime_)) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    opts_ = std::move(other.opts_);
    app_state_ = std::move(other.app_state_);
    window_state_ = std::move(other.window_state_);
    backend_runtime_ = std::move(other.backend_runtime_);
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

Result<WindowHandle> Application::create_window(const WindowOptions& options) {
  if (app_state_->run_started && window_state_->has_window) {
    return tl::unexpected(Error{"multiple_windows_unsupported",
        "only one window is supported in the first release"});
  }
  if (window_state_->has_window) {
    return tl::unexpected(Error{"multiple_windows_unsupported",
        "only one window is supported in the first release"});
  }

  if (!backend_runtime_) {
    backend_runtime_ = BackendFactory::create();
  }
  if (!backend_runtime_) {
    return tl::unexpected(Error{"unsupported_by_backend",
        "backend runtime is not available"});
  }

  auto host = backend_runtime_->create_window(app_state_, window_state_, opts_, options);
  if (!host) {
    return tl::unexpected(host.error());
  }

  window_state_->window_host = std::move(*host);
  window_state_->has_window = true;
  return WindowHandle(window_state_);
}

Result<void> Application::post(std::function<void()> task) {
  if (!backend_runtime_) {
    return tl::unexpected(Error{"invalid_state",
        "post is only valid after run() starts and before shutdown"});
  }
  return backend_runtime_->post(app_state_, std::move(task));
}

Result<int> Application::run() {
  if (!backend_runtime_) {
    return tl::unexpected(Error{"invalid_state",
        "a window must be created before run"});
  }
  return backend_runtime_->run(app_state_, window_state_);
}

} // namespace viewshell
