#include "webview_driver.h"

namespace viewshell {

bool WebviewDriver::is_allowed_url_scheme(std::string_view url) {
  return url.rfind("https://", 0) == 0 || url.rfind("http://", 0) == 0;
}

Result<void> WebviewDriver::attach(NativeWindowHandle, const WindowOptions&,
                                    const ResolvedEngine&) {
  return {};
}

Result<void> WebviewDriver::load_file(std::string_view) {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::load_url(std::string_view) {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::reload() {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::evaluate_script(std::string_view) {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::add_init_script(std::string_view) {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::open_devtools() {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::close_devtools() {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::on_page_load(PageLoadHandler) {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::set_navigation_handler(NavigationHandler) {
  return tl::unexpected(Error{"unsupported_by_backend", ""});
}

Result<void> WebviewDriver::ensure_attached() const {
  if (!attached_) {
    return tl::unexpected(Error{"invalid_state", "webview not attached"});
  }
  return {};
}

} // namespace viewshell
