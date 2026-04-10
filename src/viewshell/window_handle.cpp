#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>
#include "runtime_state.h"
#include "platform/linux_x11/window_driver.h"

namespace viewshell {

WindowHandle::WindowHandle(std::shared_ptr<RuntimeWindowState> state)
    : state_(std::move(state)) {}

Result<BridgeHandle> WindowHandle::bridge() {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", "window is closed"});
  }
  return BridgeHandle(state_);
}

Result<void> WindowHandle::set_title(std::string_view title) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->set_title(title);
  return {};
}

Result<void> WindowHandle::maximize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->maximize();
  return {};
}

Result<void> WindowHandle::unmaximize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->unmaximize();
  return {};
}

Result<void> WindowHandle::minimize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->minimize();
  return {};
}

Result<void> WindowHandle::unminimize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->unminimize();
  return {};
}

Result<void> WindowHandle::show() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->show();
  return {};
}

Result<void> WindowHandle::hide() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->hide();
  return {};
}

Result<void> WindowHandle::focus() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->focus();
  return {};
}

Result<void> WindowHandle::set_size(Size size) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->set_size(size);
  return {};
}

Result<Size> WindowHandle::get_size() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->get_size();
  return Size{};
}

Result<void> WindowHandle::set_position(Position pos) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->set_position(pos);
  return {};
}

Result<Position> WindowHandle::get_position() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->get_position();
  return Position{};
}

Result<void> WindowHandle::set_borderless(bool enabled) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->set_borderless(enabled);
  return {};
}

Result<void> WindowHandle::set_always_on_top(bool enabled) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_driver) return state_->window_driver->set_always_on_top(enabled);
  return {};
}

Result<void> WindowHandle::close() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  state_->is_closed = true;
  if (state_->window_driver) {
    return state_->window_driver->close();
  }
  return {};
}

Result<void> WindowHandle::load_url(std::string_view url) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::load_file(std::string_view entry_file) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::reload() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::evaluate_script(std::string_view script) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::add_init_script(std::string_view script) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::open_devtools() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::close_devtools() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::on_page_load(PageLoadHandler handler) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::set_navigation_handler(NavigationHandler handler) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<Capabilities> WindowHandle::capabilities() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->resolved_capabilities) {
    return *state_->resolved_capabilities;
  }
  return tl::unexpected(Error{"unsupported_by_backend", "no backend"});
}

} // namespace viewshell
