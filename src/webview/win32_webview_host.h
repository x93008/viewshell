#pragma once

#ifdef _WIN32

#include <windows.h>
#include <string_view>
#include <functional>
#include <vector>

#include <wrl.h>
#include <WebView2.h>

#include <viewshell/options.h>

namespace viewshell {

class Win32WebviewHost {
public:
  Win32WebviewHost() = default;
  ~Win32WebviewHost();

  Result<void> attach(HWND hwnd, const WindowOptions& options);
  Result<void> set_bounds(RECT bounds);
  Result<void> load_url(std::string_view url);
  Result<void> load_file(std::string_view path);
  Result<void> evaluate_script(std::string_view script);
  Result<void> add_init_script(std::string_view script);
  Result<void> open_devtools();
  Result<void> close_devtools();
  Result<void> on_page_load(PageLoadHandler handler);
  Result<void> set_navigation_handler(NavigationHandler handler);
  Result<void> set_message_handler(std::function<void(std::string_view)> handler);
  Result<void> post_json_message(std::string_view raw_message);
  void set_transparent_background(bool enabled) { transparent_background_ = enabled; }

private:
  Result<void> ensure_ready() const;
  static std::string format_hresult(HRESULT hr);

  HWND hwnd_ = nullptr;
  Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  bool com_initialized_ = false;
  bool transparent_background_ = false;
  std::function<void(std::string_view)> message_handler_;
  std::vector<std::string> init_scripts_;
  std::vector<PageLoadHandler> page_load_handlers_;
  NavigationHandler navigation_handler_;
};

} // namespace viewshell

#endif
