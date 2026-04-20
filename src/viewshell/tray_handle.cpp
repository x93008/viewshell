#include <viewshell/tray_handle.h>
#include "runtime/tray_host.h"

namespace viewshell {

TrayHandle::TrayHandle(std::shared_ptr<TrayHost> host)
    : host_(std::move(host)) {}

Result<void> TrayHandle::set_icon(std::string_view icon_path) {
  if (!host_) return tl::unexpected(Error{"invalid_state", "tray not available"});
  return host_->set_icon(icon_path);
}

Result<void> TrayHandle::set_tooltip(std::string_view tooltip) {
  if (!host_) return tl::unexpected(Error{"invalid_state", "tray not available"});
  return host_->set_tooltip(tooltip);
}

Result<void> TrayHandle::set_menu(std::vector<TrayMenuItem> menu) {
  if (!host_) return tl::unexpected(Error{"invalid_state", "tray not available"});
  return host_->set_menu(std::move(menu));
}

Result<Geometry> TrayHandle::get_icon_rect() const {
  if (!host_) return tl::unexpected(Error{"invalid_state", "tray not available"});
  return host_->get_icon_rect();
}

Result<Position> TrayHandle::get_popup_position(int popup_width, int popup_height) const {
  if (!host_) return tl::unexpected(Error{"invalid_state", "tray not available"});
  return host_->get_popup_position(popup_width, popup_height);
}

Result<void> TrayHandle::remove() {
  if (!host_) return tl::unexpected(Error{"invalid_state", "tray not available"});
  auto result = host_->remove();
  host_.reset();
  return result;
}

} // namespace viewshell
