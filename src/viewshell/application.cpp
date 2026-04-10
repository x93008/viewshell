#include <viewshell/application.h>

namespace viewshell {

Application::Application(NormalizedAppOptions opts)
    : opts_(std::move(opts)) {}

Application::Application(Application&& other) noexcept
    : opts_(std::move(other.opts_)) {}

Application& Application::operator=(Application&& other) noexcept {
  if (this != &other) {
    opts_ = std::move(other.opts_);
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

} // namespace viewshell
