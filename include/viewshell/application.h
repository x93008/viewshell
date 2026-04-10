#pragma once

#include <functional>
#include <viewshell/types.h>
#include <viewshell/options.h>
#include <viewshell/window_handle.h>

namespace viewshell {

namespace detail {
Result<NormalizedAppOptions> normalize_app_options_for_test(const AppOptions& options);
}

class Application {
public:
  static Result<Application> create(const AppOptions& options);

  Result<WindowHandle> create_window(const WindowOptions& options);
  Result<void> post(std::function<void()> task);
  Result<int> run();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) noexcept;
  Application& operator=(Application&&) noexcept;
  ~Application();

private:
  explicit Application(NormalizedAppOptions opts);

  NormalizedAppOptions opts_;
  std::shared_ptr<RuntimeWindowState> window_state_;
};

} // namespace viewshell
