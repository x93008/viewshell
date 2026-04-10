#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>
#include "runtime_state.h"

namespace viewshell {

WindowHandle::WindowHandle(std::shared_ptr<RuntimeWindowState> state)
    : state_(std::move(state)) {}

Result<BridgeHandle> WindowHandle::bridge() {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", "window is closed"});
  }
  return BridgeHandle(state_);
}

Result<void> WindowHandle::set_title(std::string_view) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::maximize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::unmaximize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::minimize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::unminimize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::show() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::hide() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::focus() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::set_size(Size) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<Size> WindowHandle::get_size() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return Size{};
}

Result<void> WindowHandle::set_position(Position) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<Position> WindowHandle::get_position() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return Position{};
}

Result<void> WindowHandle::set_borderless(bool) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::set_always_on_top(bool) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return {};
}

Result<void> WindowHandle::close() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  state_->is_closed = true;
  return {};
}

Result<void> WindowHandle::load_url(std::string_view) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::load_file(std::string_view) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::reload() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::evaluate_script(std::string_view) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::add_init_script(std::string_view) {
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

Result<void> WindowHandle::on_page_load(PageLoadHandler) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::set_navigation_handler(NavigationHandler) {
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
