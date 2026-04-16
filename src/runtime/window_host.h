#pragma once

#include <string_view>

#include <viewshell/capabilities.h>
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
};

} // namespace viewshell
