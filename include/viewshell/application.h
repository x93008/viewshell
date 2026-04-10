#pragma once

#include <functional>
#include <memory>
#include <viewshell/types.h>
#include <viewshell/options.h>
#include <viewshell/window_handle.h>

namespace viewshell {

namespace detail {
Result<NormalizedAppOptions> normalize_app_options_for_test(const AppOptions& options);
}

struct RuntimeAppState;
class WindowDriver;
class WebviewDriver;

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
  std::shared_ptr<RuntimeAppState> app_state_;
  std::shared_ptr<RuntimeWindowState> window_state_;
  std::unique_ptr<WindowDriver> window_driver_;
  std::unique_ptr<WebviewDriver> webview_driver_;

  friend void MarkRunStartedForTest(Application&);
  friend void MarkShutdownStartedForTest(Application&);
  friend void EnterRunLoopForTest(Application&);
  friend Result<int> FinishRunForTest(Application&);
  friend void PumpPostedTasksForTest(Application&);
  friend std::vector<std::string> TakeRuntimeLogsForTest(Application&);
};

} // namespace viewshell
