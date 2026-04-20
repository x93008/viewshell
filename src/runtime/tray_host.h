#pragma once

#include <string_view>
#include <vector>

#include <viewshell/types.h>
#include <viewshell/tray_options.h>

namespace viewshell {

class TrayHost {
public:
  virtual ~TrayHost() = default;

  virtual Result<void> set_icon(std::string_view icon_path) = 0;
  virtual Result<void> set_tooltip(std::string_view tooltip) = 0;
  virtual Result<void> set_menu(std::vector<TrayMenuItem> menu) = 0;
  virtual Result<Geometry> get_icon_rect() const = 0;
  virtual Result<Position> get_popup_position(int popup_width, int popup_height) const = 0;
  virtual Result<void> remove() = 0;
};

} // namespace viewshell
