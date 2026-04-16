#pragma once

#ifdef _WIN32

#include <memory>
#include <unordered_set>
#include <windows.h>

#include <viewshell/options.h>

#include "runtime/window_host.h"

namespace viewshell { class Win32WebviewHost; class InvokeBus; }

namespace viewshell {

struct RuntimeAppState;
struct RuntimeWindowState;

class Win32WindowHost final : public WindowHost {
public:
  static Result<std::shared_ptr<Win32WindowHost>> create(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const WindowOptions& options);

  ~Win32WindowHost() override;

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

private:
  Win32WindowHost(std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state);

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
  Result<void> ensure_window() const;
  void update_style();
  RECT client_rect() const;
  void update_shape();

  std::weak_ptr<RuntimeAppState> app_state_;
  std::weak_ptr<RuntimeWindowState> window_state_;
  std::unique_ptr<Win32WebviewHost> webview_host_;
  std::unique_ptr<InvokeBus> invoke_bus_;
  std::unordered_set<std::string> subscribed_events_;
  HWND hwnd_ = nullptr;
  bool borderless_ = false;
  bool always_on_top_ = false;
  bool show_in_taskbar_ = true;
  bool resizable_ = true;
  Size size_{};
  Position position_{};
};

} // namespace viewshell

#endif
