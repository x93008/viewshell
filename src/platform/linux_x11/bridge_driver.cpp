#include "bridge_driver.h"
#include "webview_driver.h"

namespace viewshell {

Result<void> BridgeDriver::attach(WebviewDriver& webview) {
  webview_ = &webview;
  return {};
}

Result<void> BridgeDriver::post_to_page(std::string_view raw_message) {
  if (!bridge_ready_) {
    return tl::unexpected(Error{"invalid_state", "bridge not ready"});
  }
  last_posted_ = std::string(raw_message);
  return {};
}

} // namespace viewshell
