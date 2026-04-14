#pragma once

#ifdef _WIN32

#include <windows.h>
#include <string_view>

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

private:
  Result<void> ensure_ready() const;
  static std::string format_hresult(HRESULT hr);

  HWND hwnd_ = nullptr;
  Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  bool com_initialized_ = false;
};

} // namespace viewshell

#endif
