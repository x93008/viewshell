#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>
#include "runtime_state.h"
#include "runtime/window_host.h"

namespace viewshell {

Result<BridgeHandle> WindowHandle::bridge() {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", "window is closed"});
  }
  return BridgeHandle(state_);
}

Result<void> WindowHandle::set_title(std::string_view title) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_title(title);
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::maximize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->maximize();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::unmaximize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->unmaximize();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::minimize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->minimize();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::unminimize() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->unminimize();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::show() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->show();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::hide() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->hide();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::focus() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->focus();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::set_geometry(Geometry geometry) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_geometry(geometry);
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<Geometry> WindowHandle::get_geometry() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->get_geometry();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::set_size(Size size) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_size(size);
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<Size> WindowHandle::get_size() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->get_size();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::set_position(Position pos) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_position(pos);
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<Position> WindowHandle::get_position() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->get_position();
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::set_borderless(bool enabled) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_borderless(enabled);
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::set_always_on_top(bool enabled) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_always_on_top(enabled);
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::close() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  state_->is_closed = true;
  if (state_->window_host) {
    return state_->window_host->close();
  }
  return tl::unexpected(Error{"unsupported_by_backend", "no window backend"});
}

Result<void> WindowHandle::load_url(std::string_view url) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->load_url(url);
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::load_file(std::string_view entry_file) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->load_file(entry_file);
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::reload() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->reload();
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::evaluate_script(std::string_view script) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->evaluate_script(script);
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::add_init_script(std::string_view script) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->add_init_script(script);
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::open_devtools() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->open_devtools();
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::close_devtools() {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->close_devtools();
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::on_page_load(PageLoadHandler handler) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->on_page_load(std::move(handler));
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<void> WindowHandle::set_navigation_handler(NavigationHandler handler) {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->window_host) return state_->window_host->set_navigation_handler(std::move(handler));
  return tl::unexpected(Error{"unsupported_by_backend", "no webview backend"});
}

Result<Capabilities> WindowHandle::capabilities() const {
  if (state_->is_closed) return tl::unexpected(Error{"window_closed", ""});
  if (state_->resolved_capabilities) {
    return *state_->resolved_capabilities;
  }
  if (state_->window_host) {
    return state_->window_host->capabilities();
  }
  return tl::unexpected(Error{"unsupported_by_backend", "no backend"});
}

} // namespace viewshell
