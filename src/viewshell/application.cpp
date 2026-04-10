#include <viewshell/application.h>

namespace viewshell {

Application::Application(int bridge_timeout_ms)
    : bridge_timeout_ms_(bridge_timeout_ms) {}

Application::Application(Application&& other) noexcept
    : bridge_timeout_ms_(other.bridge_timeout_ms_) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    bridge_timeout_ms_ = other.bridge_timeout_ms_;
  }
  return *this;
}

Application::~Application() = default;

Result<Application> Application::create(const AppOptions& options) {
  if (options.bridge_timeout_ms <= 0) {
    return tl::unexpected(Error{"invalid_config",
        "bridge_timeout_ms must be a positive integer"});
  }
  return Application(options.bridge_timeout_ms);
}

} // namespace viewshell
