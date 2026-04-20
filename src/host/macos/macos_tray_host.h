#pragma once

#ifdef __APPLE__

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <viewshell/tray_options.h>
#include <viewshell/types.h>

#include "runtime/tray_host.h"

namespace viewshell {

class MacOSTrayHost final : public TrayHost {
public:
  static Result<std::shared_ptr<MacOSTrayHost>> create(const TrayOptions& options);

  ~MacOSTrayHost() override;

  Result<void> set_icon(std::string_view icon_path) override;
  Result<void> set_tooltip(std::string_view tooltip) override;
  Result<void> set_menu(std::vector<TrayMenuItem> menu) override;
  Result<Geometry> get_icon_rect() const override;
  Result<Position> get_popup_position(int popup_width, int popup_height) const override;
  Result<void> remove() override;

  // Called by ViewshellTrayDelegate
  void* status_item_ptr() const;
  void* menu_ptr() const;
  void invoke_click();
  void invoke_right_click();
  void invoke_menu_click(int index);

private:
  MacOSTrayHost() = default;

  void rebuild_menu();

  void* status_item_ = nullptr;
  void* menu_ = nullptr;
  void* delegate_ = nullptr;

  std::vector<TrayMenuItem> menu_items_;
  std::function<void()> on_click_;
  std::function<void()> on_right_click_;
  std::function<void(const std::string&)> on_menu_click_;
};

} // namespace viewshell

#endif
