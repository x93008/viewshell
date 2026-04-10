#include <viewshell/application.h>
#include "runtime_state.h"
#include "platform/linux_x11/window_driver.h"
#include "platform/linux_x11/webview_driver.h"

namespace viewshell {

Application::Application(NormalizedAppOptions opts)
    : opts_(std::move(opts)),
      app_state_(std::make_shared<RuntimeAppState>()),
      window_state_(std::make_shared<RuntimeWindowState>()) {}

Application::Application(Application&& other) noexcept
    : opts_(std::move(other.opts_)),
      app_state_(std::move(other.app_state_)),
      window_state_(std::move(other.window_state_)),
      window_driver_(std::move(other.window_driver_)),
      webview_driver_(std::move(other.webview_driver_)) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    opts_ = std::move(other.opts_);
    app_state_ = std::move(other.app_state_);
    window_state_ = std::move(other.window_state_);
    window_driver_ = std::move(other.window_driver_);
    webview_driver_ = std::move(other.webview_driver_);
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

  auto driver = std::make_unique<WindowDriver>();
  auto native = driver->create(options);
  if (!native) {
    return tl::unexpected(native.error());
  }

  window_state_->window_driver = driver.get();
  window_driver_ = std::move(driver);

  window_state_->window_driver->on_close = [this]() {
    app_state_->shutdown_started = true;
    app_state_->run_exit_code = 0;
    window_state_->is_closed = true;
    window_state_->window_driver->quit_main_loop();
  };

  auto wv = std::make_unique<WebviewDriver>();
  auto attach_result = wv->attach(*native, options);
  if (!attach_result) {
    return tl::unexpected(attach_result.error());
  }

  window_state_->webview_driver = wv.get();
  webview_driver_ = std::move(wv);

  if (options.asset_root.has_value() && !options.asset_root->empty()) {
    auto load_result = window_state_->webview_driver->load_file(*options.asset_root);
    if (!load_result) {
      return tl::unexpected(load_result.error());
    }
  }

  window_state_->has_window = true;
  window_state_->window_driver->show();
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
  app_state_->run_started = true;

  if (window_state_->window_driver) {
    window_state_->window_driver->run_main_loop();
  }

  return app_state_->run_exit_code;
}

} // namespace viewshell
