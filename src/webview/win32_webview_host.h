#pragma once

#ifdef _WIN32

#include <windows.h>
#include <string_view>

#if VIEWSHELL_HAS_WEBVIEW2
#include <wrl.h>
#include <WebView2.h>
#endif

#include <viewshell/options.h>

namespace viewshell {

class Win32WebviewHost {
public:
  Win32WebviewHost() = default;

  Result<void> attach(HWND hwnd, const WindowOptions& options);
  Result<void> set_bounds(RECT bounds);
  Result<void> load_url(std::string_view url);
  Result<void> evaluate_script(std::string_view script);

private:
  Result<void> ensure_ready() const;

  HWND hwnd_ = nullptr;
#if VIEWSHELL_HAS_WEBVIEW2
  Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
#endif
};

} // namespace viewshell

#endif
