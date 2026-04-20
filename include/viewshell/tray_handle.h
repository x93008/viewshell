#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <viewshell/types.h>
#include <viewshell/tray_options.h>

namespace viewshell {

class TrayHost;

class TrayHandle {
public:
  TrayHandle() = default;
  explicit TrayHandle(std::shared_ptr<TrayHost> host);

  Result<void> set_icon(std::string_view icon_path);
  Result<void> set_tooltip(std::string_view tooltip);
  Result<void> set_menu(std::vector<TrayMenuItem> menu);
  Result<Geometry> get_icon_rect() const;
  Result<Position> get_popup_position(int popup_width, int popup_height) const;
  Result<void> remove();

private:
  std::shared_ptr<TrayHost> host_;
};

} // namespace viewshell
