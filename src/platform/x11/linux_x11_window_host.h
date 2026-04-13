#pragma once

#include <memory>
#include <unordered_set>

#include <viewshell/options.h>

#include "runtime/window_host.h"

namespace viewshell {

struct RuntimeAppState;
struct RuntimeWindowState;
class BridgeDriver;
class InvokeBus;
class WindowDriver;
class WebviewDriver;

class LinuxX11WindowHost final : public WindowHost {
public:
  static Result<std::shared_ptr<LinuxX11WindowHost>> create(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const WindowOptions& options);

  ~LinuxX11WindowHost() override;

  void run_main_loop();

  Result<void> set_title(std::string_view title) override;
  Result<void> maximize() override;
  Result<void> unmaximize() override;
  Result<void> minimize() override;
  Result<void> unminimize() override;
  Result<void> show() override;
  Result<void> hide() override;
  Result<void> focus() override;
  Result<void> set_size(Size size) override;
  Result<Size> get_size() const override;
  Result<void> set_position(Position pos) override;
  Result<Position> get_position() const override;
  Result<void> set_borderless(bool enabled) override;
  Result<void> set_always_on_top(bool enabled) override;
  Result<void> close() override;
  Result<void> load_url(std::string_view url) override;
  Result<void> load_file(std::string_view entry_file) override;
  Result<void> reload() override;
  Result<void> evaluate_script(std::string_view script) override;
  Result<void> add_init_script(std::string_view script) override;
  Result<void> open_devtools() override;
  Result<void> close_devtools() override;
  Result<void> on_page_load(PageLoadHandler handler) override;
  Result<void> set_navigation_handler(NavigationHandler handler) override;
  Result<Capabilities> capabilities() const override;
  Result<void> register_command(std::string name, CommandHandler handler) override;
  Result<void> emit(std::string name, const Json& payload) override;

private:
  LinuxX11WindowHost(std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state);

  std::weak_ptr<RuntimeAppState> app_state_;
  std::weak_ptr<RuntimeWindowState> window_state_;
  std::unique_ptr<BridgeDriver> bridge_driver_;
  std::unique_ptr<InvokeBus> invoke_bus_;
  std::unique_ptr<WindowDriver> window_driver_;
  std::unique_ptr<WebviewDriver> webview_driver_;
  std::unordered_set<std::string> subscribed_events_;
};

} // namespace viewshell
