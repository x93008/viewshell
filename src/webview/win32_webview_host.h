#pragma once

#ifdef _WIN32

#include <wrl.h>
#include <string_view>

#include <WebView2.h>

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
  Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
};

} // namespace viewshell

#endif
