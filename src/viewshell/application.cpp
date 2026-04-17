#include <viewshell/application.h>
#include "runtime_state.h"
#include "runtime/backend_factory.h"
#include "runtime/backend_runtime.h"

namespace viewshell {

Application::Application(NormalizedAppOptions opts)
    : opts_(std::move(opts)),
      app_state_(std::make_shared<RuntimeAppState>()),
      backend_runtime_(BackendFactory::create()) {}

Application::Application(Application&& other) noexcept
    : opts_(std::move(other.opts_)),
      app_state_(std::move(other.app_state_)),
      backend_runtime_(std::move(other.backend_runtime_)) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    opts_ = std::move(other.opts_);
    app_state_ = std::move(other.app_state_);
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
  if (!backend_runtime_) {
    backend_runtime_ = BackendFactory::create();
  }
  if (!backend_runtime_) {
    return tl::unexpected(Error{"unsupported_by_backend",
        "backend runtime is not available"});
  }

  auto window_state = std::make_shared<RuntimeWindowState>();
  auto host = backend_runtime_->create_window(app_state_, window_state, opts_, options);
  if (!host) {
    return tl::unexpected(host.error());
  }

  window_state->window_host = std::move(*host);
  app_state_->windows.push_back(window_state);
  return WindowHandle(window_state);
}

Result<TrayHandle> Application::create_tray(const TrayOptions& options) {
  if (!backend_runtime_) {
    backend_runtime_ = BackendFactory::create();
  }
  if (!backend_runtime_) {
    return tl::unexpected(Error{"unsupported_by_backend",
        "backend runtime is not available"});
  }
  if (app_state_->tray) {
    return tl::unexpected(Error{"invalid_state",
        "a system tray already exists"});
  }

  auto host = backend_runtime_->create_tray(options);
  if (!host) {
    return tl::unexpected(host.error());
  }

  app_state_->tray = *host;
  return TrayHandle(*host);
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
  if (app_state_->windows.empty()) {
    return tl::unexpected(Error{"invalid_state",
        "a window must be created before run"});
  }
  return backend_runtime_->run(app_state_);
}

} // namespace viewshell
