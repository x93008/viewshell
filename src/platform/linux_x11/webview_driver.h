#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <optional>
#include <filesystem>
#include <memory>
#include <viewshell/types.h>
#include <viewshell/options.h>
#include <viewshell/capabilities.h>
#include "native_window_handle.h"
#include "resource_protocol.h"

typedef struct _WebKitWebView WebKitWebView;
typedef struct _WebKitUserContentManager WebKitUserContentManager;

namespace viewshell {

class WebviewDriver {
public:
  WebviewDriver();
  ~WebviewDriver();

  Result<void> attach(NativeWindowHandle native, const WindowOptions& options);
  Result<void> load_file(std::string_view entry_file);
  Result<void> load_url(std::string_view url);
  Result<void> reload();
  Result<void> evaluate_script(std::string_view script);
  Result<void> add_init_script(std::string_view script);
  Result<void> open_devtools();
  Result<void> close_devtools();
  Result<void> on_page_load(PageLoadHandler handler);
  Result<void> set_navigation_handler(NavigationHandler handler);

  const Capabilities& capabilities() const { return capabilities_; }

  std::function<void()> on_close;

  static bool is_allowed_url_scheme(std::string_view url);

private:
  Result<void> ensure_attached() const;

  WebKitWebView* webview_ = nullptr;
  WebKitUserContentManager* user_content_manager_ = nullptr;
  bool attached_ = false;
  Capabilities capabilities_;
  std::vector<PageLoadHandler> page_load_handlers_;
  NavigationHandler navigation_handler_;
  std::vector<std::string> init_scripts_;
  std::unique_ptr<ResourceProtocol> resource_protocol_;
};

} // namespace viewshell
