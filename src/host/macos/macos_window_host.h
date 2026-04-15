#pragma once

#ifdef __APPLE__

#include <memory>
#include <unordered_set>
#include <vector>

#include <viewshell/options.h>

#include "runtime/window_host.h"

namespace viewshell {

struct RuntimeAppState;
struct RuntimeWindowState;
class InvokeBus;

class MacOSWindowHost final : public WindowHost {
public:
  static Result<std::shared_ptr<MacOSWindowHost>> create(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const WindowOptions& options);

  ~MacOSWindowHost() override;

  void run_message_loop();

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

  void handle_script_message(std::string_view message);
  void dispatch_json_to_page(const Json& payload);
  void begin_drag();

private:
  MacOSWindowHost(std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state);
  Result<void> ensure_window() const;
  void update_style();

  std::weak_ptr<RuntimeAppState> app_state_;
  std::weak_ptr<RuntimeWindowState> window_state_;
  void* window_ = nullptr;
  void* delegate_ = nullptr;
  void* webview_ = nullptr;
  void* message_handler_ = nullptr;
  void* user_content_controller_ = nullptr;
  std::unique_ptr<InvokeBus> invoke_bus_;
  std::unordered_set<std::string> subscribed_events_;
  std::vector<std::string> init_scripts_;
  bool borderless_ = false;
  bool always_on_top_ = false;
};

} // namespace viewshell

#endif
