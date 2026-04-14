#ifdef _WIN32

#include "webview/win32_webview_host.h"

#include <windows.h>
#include <objbase.h>
#include <filesystem>
#include <sstream>
#include <atomic>
#include <string>

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

std::wstring to_file_url(std::string_view path) {
  auto absolute = std::filesystem::absolute(std::filesystem::path(std::string(path)));
  auto generic = absolute.generic_u8string();
  return to_wstring("file:///" + generic);
}

void pump_pending_messages() {
  MSG msg;
  while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

} // namespace

Win32WebviewHost::~Win32WebviewHost() {
  if (com_initialized_) {
    CoUninitialize();
  }
}

std::string Win32WebviewHost::format_hresult(HRESULT hr) {
  std::ostringstream out;
  out << "HRESULT=0x" << std::hex << static_cast<unsigned long>(hr);
  return out.str();
}

Result<void> Win32WebviewHost::attach(HWND hwnd, const WindowOptions&) {
  hwnd_ = hwnd;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    com_initialized_ = true;
  } else if (hr != RPC_E_CHANGED_MODE) {
    return tl::unexpected(Error{"engine_init_failed",
        "failed to initialize COM for WebView2: " + format_hresult(hr)});
  }

  std::atomic<bool> env_done = false;
  HRESULT env_result = E_FAIL;
  hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, nullptr,
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this, &env_done, &env_result](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            if (SUCCEEDED(result) && env) {
              environment_ = env;
            }
            env_result = result;
            env_done = true;
            return S_OK;
          }).Get());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"engine_init_failed",
        "failed to create WebView2 environment: " + format_hresult(hr)});
  }
  while (!env_done) {
    pump_pending_messages();
    Sleep(1);
  }
  hr = env_result;
  if (FAILED(hr) || !environment_) {
    return tl::unexpected(Error{"engine_init_failed",
        "failed to initialize WebView2 environment: " + format_hresult(hr)});
  }

  std::atomic<bool> controller_done = false;
  HRESULT controller_result = E_FAIL;
  hr = environment_->CreateCoreWebView2Controller(
      hwnd_,
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [this, &controller_done, &controller_result](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            if (SUCCEEDED(result) && controller) {
              controller_ = controller;
              controller_->get_CoreWebView2(&webview_);
            }
            controller_result = result;
            controller_done = true;
            return S_OK;
          }).Get());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"engine_init_failed",
        "failed to create WebView2 controller: " + format_hresult(hr)});
  }
  while (!controller_done) {
    pump_pending_messages();
    Sleep(1);
  }
  hr = controller_result;
  if (FAILED(hr) || !controller_ || !webview_) {
    return tl::unexpected(Error{"engine_init_failed",
        "failed to initialize WebView2 controller: " + format_hresult(hr)});
  }

  RECT bounds{};
  GetClientRect(hwnd_, &bounds);
  controller_->put_Bounds(bounds);
  return {};
}

Result<void> Win32WebviewHost::ensure_ready() const {
  if (!controller_ || !webview_) {
    return tl::unexpected(Error{"unsupported_by_backend", "windows webview backend not initialized"});
  }
  return {};
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
    return tl::unexpected(Error{"invalid_state",
        "failed to navigate WebView2: " + format_hresult(hr)});
  }
  return {};
}

Result<void> Win32WebviewHost::load_file(std::string_view path) {
  if (auto result = ensure_ready(); !result) return result;
  auto file_url = to_file_url(path);
  HRESULT hr = webview_->Navigate(file_url.c_str());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state",
        "failed to navigate local file in WebView2: " + format_hresult(hr)});
  }
  return {};
}

Result<void> Win32WebviewHost::evaluate_script(std::string_view script) {
  if (auto result = ensure_ready(); !result) return result;
  auto wide = to_wstring(script);
  HRESULT hr = webview_->ExecuteScript(wide.c_str(), nullptr);
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state",
        "failed to execute script in WebView2: " + format_hresult(hr)});
  }
  return {};
}

} // namespace viewshell

#endif
