#pragma once

#include <string>
#include <string_view>
#include <unordered_set>

#include <viewshell/capabilities.h>
#include <viewshell/options.h>
#include <viewshell/types.h>

namespace viewshell {

class WindowHost {
public:
  virtual ~WindowHost() = default;

  virtual Result<void> set_title(std::string_view title) = 0;
  virtual Result<void> maximize() = 0;
  virtual Result<void> unmaximize() = 0;
  virtual Result<void> minimize() = 0;
  virtual Result<void> unminimize() = 0;
  virtual Result<void> show() = 0;
  virtual Result<void> hide() = 0;
  virtual Result<void> focus() = 0;
  virtual Result<void> set_geometry(Geometry geometry) = 0;
  virtual Result<Geometry> get_geometry() const = 0;
  virtual Result<void> set_size(Size size) = 0;
  virtual Result<Size> get_size() const = 0;
  virtual Result<void> set_position(Position pos) = 0;
  virtual Result<Position> get_position() const = 0;
  virtual Result<void> set_borderless(bool enabled) = 0;
  virtual Result<void> set_always_on_top(bool enabled) = 0;
  virtual Result<void> close() = 0;
  virtual Result<void> load_url(std::string_view url) = 0;
  virtual Result<void> load_file(std::string_view entry_file) = 0;
  virtual Result<void> reload() = 0;
  virtual Result<void> evaluate_script(std::string_view script) = 0;
  virtual Result<void> add_init_script(std::string_view script) = 0;
  virtual Result<void> open_devtools() = 0;
  virtual Result<void> close_devtools() = 0;
  virtual Result<void> on_page_load(PageLoadHandler handler) = 0;
  virtual Result<void> set_navigation_handler(NavigationHandler handler) = 0;
  virtual Result<Capabilities> capabilities() const = 0;
  virtual Result<void> register_command(std::string name, CommandHandler handler) = 0;
  virtual Result<void> emit(std::string name, const Json& payload) = 0;
  virtual void begin_drag() {}

  // Handle built-in __wnd.* invoke commands.
  // Returns true if the command was handled, false if not a __wnd command.
  bool handle_wnd_command(const std::string& name, const Json& payload, Json& out_payload, Result<void>& out_result);

protected:
  void apply_common_options(const WindowOptions& options);

  bool borderless_ = false;
  bool always_on_top_ = false;
  bool show_in_taskbar_ = true;
  bool resizable_ = true;
  bool inject_window_api_ = false;
  std::unordered_set<std::string> subscribed_events_;
};

} // namespace viewshell
