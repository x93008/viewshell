#ifdef _WIN32

#include "webview/win32_webview_host.h"

#include <string>

#if VIEWSHELL_HAS_WEBVIEW2
#include <future>
#endif

namespace viewshell {

namespace {

std::wstring to_wstring(std::string_view value) {
  if (value.empty()) {
    return {};
  }
  int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring out(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), out.data(), size);
  return out;
}

} // namespace

Result<void> Win32WebviewHost::attach(HWND hwnd, const WindowOptions&) {
  hwnd_ = hwnd;

#if !VIEWSHELL_HAS_WEBVIEW2
  return tl::unexpected(Error{"unsupported_by_backend", "WebView2 SDK not available at build time"});
#else

  std::promise<HRESULT> env_promise;
  auto env_future = env_promise.get_future();
  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, nullptr,
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this, &env_promise](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            if (SUCCEEDED(result) && env) {
              environment_ = env;
            }
            env_promise.set_value(result);
            return S_OK;
          }).Get());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"engine_init_failed", "failed to create WebView2 environment"});
  }
  hr = env_future.get();
  if (FAILED(hr) || !environment_) {
    return tl::unexpected(Error{"engine_init_failed", "failed to initialize WebView2 environment"});
  }

  std::promise<HRESULT> controller_promise;
  auto controller_future = controller_promise.get_future();
  hr = environment_->CreateCoreWebView2Controller(
      hwnd_,
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [this, &controller_promise](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            if (SUCCEEDED(result) && controller) {
              controller_ = controller;
              controller_->get_CoreWebView2(&webview_);
            }
            controller_promise.set_value(result);
            return S_OK;
          }).Get());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"engine_init_failed", "failed to create WebView2 controller"});
  }
  hr = controller_future.get();
  if (FAILED(hr) || !controller_ || !webview_) {
    return tl::unexpected(Error{"engine_init_failed", "failed to initialize WebView2 controller"});
  }

  RECT bounds{};
  GetClientRect(hwnd_, &bounds);
  controller_->put_Bounds(bounds);
  return {};
#endif
}

Result<void> Win32WebviewHost::ensure_ready() const {
#if !VIEWSHELL_HAS_WEBVIEW2
  return tl::unexpected(Error{"unsupported_by_backend", "WebView2 SDK not available at build time"});
#else
  if (!controller_ || !webview_) {
    return tl::unexpected(Error{"unsupported_by_backend", "windows webview backend not initialized"});
  }
  return {};
#endif
}

Result<void> Win32WebviewHost::set_bounds(RECT bounds) {
  if (auto result = ensure_ready(); !result) return result;
  controller_->put_Bounds(bounds);
  return {};
}

Result<void> Win32WebviewHost::load_url(std::string_view url) {
  if (auto result = ensure_ready(); !result) return result;
  auto wide = to_wstring(url);
  HRESULT hr = webview_->Navigate(wide.c_str());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state", "failed to navigate WebView2"});
  }
  return {};
}

Result<void> Win32WebviewHost::evaluate_script(std::string_view script) {
  if (auto result = ensure_ready(); !result) return result;
  auto wide = to_wstring(script);
  HRESULT hr = webview_->ExecuteScript(wide.c_str(), nullptr);
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state", "failed to execute script in WebView2"});
  }
  return {};
}

} // namespace viewshell

#endif
