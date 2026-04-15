#ifdef _WIN32

#include "webview/win32_webview_host.h"

#include <windows.h>
#include <objbase.h>
#include <filesystem>
#include <sstream>
#include <atomic>
#include <string>

#include <shlobj.h>

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

std::wstring webview2_user_data_dir(HWND hwnd) {
  PWSTR local_app_data = nullptr;
  std::wstring result;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data))) {
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    std::filesystem::path exe(exe_path);
    auto dir = std::filesystem::path(local_app_data) / L"Viewshell" / exe.stem() / L"WebView2";
    std::filesystem::create_directories(dir);
    result = dir.wstring();
    CoTaskMemFree(local_app_data);
  }
  return result;
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
  auto user_data = webview2_user_data_dir(hwnd_);
  hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data.empty() ? nullptr : user_data.c_str(), nullptr,
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

  if (transparent_background_) {
    Microsoft::WRL::ComPtr<ICoreWebView2Controller2> controller2;
    if (SUCCEEDED(controller_.As(&controller2)) && controller2) {
      COREWEBVIEW2_COLOR color{};
      color.A = 0;
      color.R = 0;
      color.G = 0;
      color.B = 0;
      controller2->put_DefaultBackgroundColor(color);
    }
  }

  webview_->add_NavigationStarting(
      Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
          [this](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
            LPWSTR uri = nullptr;
            args->get_Uri(&uri);
            std::wstring wide = uri ? uri : L"";
            if (uri) CoTaskMemFree(uri);
            int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string utf8(size > 0 ? size - 1 : 0, '\0');
            if (size > 1) {
              WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), size - 1, nullptr, nullptr);
            }
            PageLoadEvent event{utf8, "started", std::nullopt};
            for (auto& handler : page_load_handlers_) {
              handler(event);
            }
            return S_OK;
          }).Get(),
      nullptr);

  webview_->add_NavigationCompleted(
      Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            LPWSTR uri = nullptr;
            sender->get_Source(&uri);
            std::wstring wide = uri ? uri : L"";
            if (uri) CoTaskMemFree(uri);
            int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string utf8(size > 0 ? size - 1 : 0, '\0');
            if (size > 1) {
              WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), size - 1, nullptr, nullptr);
            }
            BOOL success = FALSE;
            args->get_IsSuccess(&success);
            COREWEBVIEW2_WEB_ERROR_STATUS status{};
            args->get_WebErrorStatus(&status);
            PageLoadEvent event{utf8, "finished", success ? std::nullopt : std::optional<std::string>(std::to_string(static_cast<int>(status)))};
            for (auto& handler : page_load_handlers_) {
              handler(event);
            }
            return S_OK;
          }).Get(),
      nullptr);

  for (const auto& script : init_scripts_) {
    auto wide = to_wstring(script);
    webview_->AddScriptToExecuteOnDocumentCreated(wide.c_str(), nullptr);
  }

  if (message_handler_) {
    webview_->add_WebMessageReceived(
        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
              LPWSTR message = nullptr;
              if (FAILED(args->TryGetWebMessageAsString(&message)) || !message) {
                return S_OK;
              }
              int size = WideCharToMultiByte(CP_UTF8, 0, message, -1, nullptr, 0, nullptr, nullptr);
              std::string utf8(size > 0 ? size - 1 : 0, '\0');
              if (size > 1) {
                WideCharToMultiByte(CP_UTF8, 0, message, -1, utf8.data(), size - 1, nullptr, nullptr);
              }
              CoTaskMemFree(message);
              if (message_handler_) {
                message_handler_(utf8);
              }
              return S_OK;
            }).Get(),
        nullptr);
  }

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

Result<void> Win32WebviewHost::add_init_script(std::string_view script) {
  init_scripts_.push_back(std::string(script));
  if (controller_ && webview_) {
    auto wide = to_wstring(script);
    HRESULT hr = webview_->AddScriptToExecuteOnDocumentCreated(wide.c_str(), nullptr);
    if (FAILED(hr)) {
      return tl::unexpected(Error{"invalid_state",
          "failed to add init script in WebView2: " + format_hresult(hr)});
    }

    // If a page is already loaded, mirror the init script into the current document
    // so callers that register scripts after create_window() can still observe it.
    hr = webview_->ExecuteScript(wide.c_str(), nullptr);
    if (FAILED(hr)) {
      return tl::unexpected(Error{"invalid_state",
          "failed to execute current-page init script in WebView2: " + format_hresult(hr)});
    }
  }
  return {};
}

Result<void> Win32WebviewHost::open_devtools() {
  if (auto result = ensure_ready(); !result) return result;
  HRESULT hr = webview_->OpenDevToolsWindow();
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state",
        "failed to open WebView2 devtools: " + format_hresult(hr)});
  }
  return {};
}

Result<void> Win32WebviewHost::close_devtools() {
  if (auto result = ensure_ready(); !result) return result;
  auto method = to_wstring("Browser.close");
  auto params = to_wstring("{}");
  HRESULT hr = webview_->CallDevToolsProtocolMethod(method.c_str(), params.c_str(), nullptr);
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state",
        "failed to close WebView2 devtools: " + format_hresult(hr)});
  }
  return {};
}

Result<void> Win32WebviewHost::on_page_load(PageLoadHandler handler) {
  page_load_handlers_.push_back(std::move(handler));
  return {};
}

Result<void> Win32WebviewHost::set_message_handler(std::function<void(std::string_view)> handler) {
  message_handler_ = std::move(handler);
  return {};
}

Result<void> Win32WebviewHost::post_json_message(std::string_view raw_message) {
  if (auto result = ensure_ready(); !result) return result;
  auto wide = to_wstring(raw_message);
  HRESULT hr = webview_->PostWebMessageAsJson(wide.c_str());
  if (FAILED(hr)) {
    return tl::unexpected(Error{"invalid_state",
        "failed to post WebView2 web message: " + format_hresult(hr)});
  }
  return {};
}

} // namespace viewshell

#endif
