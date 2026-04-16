#ifdef _WIN32

#include "win32_window_host.h"

#include <dwmapi.h>
#include <nlohmann/json.hpp>
#include <string>

#pragma comment(lib, "dwmapi.lib")

#include "bridge/invoke_bus.h"
#include "webview/win32_webview_host.h"
#include "viewshell/runtime_state.h"

namespace viewshell {

namespace {

bool all_windows_closed(const std::shared_ptr<RuntimeAppState>& app_state) {
  for (const auto& window : app_state->windows) {
    if (window && !window->is_closed) {
      return false;
    }
  }
  return true;
}

std::wstring to_wstring(std::string_view value) {
  if (value.empty()) {
    return {};
  }
  int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring out(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), out.data(), size);
  return out;
}

Error unsupported_webview_error() {
  return Error{"unsupported_by_backend", "windows webview backend not implemented yet"};
}

constexpr const char* kWindowsBridgeBootstrap = R"JS((function () {
  if (!window.chrome || !window.chrome.webview || window.__viewshell) return;

  var nextRequestId = 1;
  var pending = new Map();
  var listeners = new Map();

  function post(kind, name, payload, requestId) {
    window.chrome.webview.postMessage(JSON.stringify({
      kind: kind,
      name: name || null,
      payload: payload || {},
      requestId: requestId || null
    }));
  }

  window.chrome.webview.addEventListener('message', function (event) {
    var detail = event && event.data ? event.data : {};
    window.dispatchEvent(new CustomEvent('viewshell:message', { detail: detail }));

    if (detail.kind === 'invoke_result' && detail.requestId != null) {
      var entry = pending.get(detail.requestId);
      if (!entry) return;
      pending.delete(detail.requestId);
      if (detail.ok) {
        entry.resolve(detail.payload || {});
      } else {
        entry.reject(detail.error || { code: 'bridge_error', message: 'unknown bridge error' });
      }
      return;
    }

    if (detail.kind === 'native_event') {
      var entries = listeners.get(detail.name) || [];
      entries.slice().forEach(function (listener) {
        listener(detail.payload || {});
      });
    }
  });

  window.__viewshell = {
    invoke: function (name, args) {
      var requestId = nextRequestId++;
      return new Promise(function (resolve, reject) {
        pending.set(requestId, { resolve: resolve, reject: reject });
        post('invoke', name, args || {}, requestId);
      });
    },
    emit: function (name, payload) {
      post('emit', name, payload || {}, null);
    },
    on: function (name, listener) {
      var entries = listeners.get(name) || [];
      var wasEmpty = entries.length === 0;
      entries.push(listener);
      listeners.set(name, entries);
      if (wasEmpty) post('subscribe', name, {}, null);
      return function () { window.__viewshell.off(name, listener); };
    },
    off: function (name, listener) {
      var entries = listeners.get(name) || [];
      var next = entries.filter(function (entry) { return entry !== listener; });
      listeners.set(name, next);
      if (entries.length > 0 && next.length === 0) post('unsubscribe', name, {}, null);
    }
  };

  window.webkit = window.webkit || { messageHandlers: {} };
  window.webkit.messageHandlers.viewshellDrag = {
    postMessage: function (payload) {
      post('drag', null, { value: payload || 'drag' }, null);
    }
  };
  window.__viewshellWinClose = function () {
    post('close', null, {}, null);
  };
})();)JS";

} // namespace

Win32WindowHost::Win32WindowHost(std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state)
    : app_state_(std::move(app_state)),
      window_state_(std::move(window_state)) {}

Win32WindowHost::~Win32WindowHost() {
  if (hwnd_) {
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}

LRESULT CALLBACK Win32WindowHost::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_NCCREATE) {
    auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
    auto* self = static_cast<Win32WindowHost*>(create->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    return DefWindowProcW(hwnd, message, wparam, lparam);
  }

  auto* self = reinterpret_cast<Win32WindowHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) {
    return DefWindowProcW(hwnd, message, wparam, lparam);
  }

  if (message == WM_CLOSE) {
    if (auto app = self->app_state_.lock()) {
      if (auto window = self->window_state_.lock()) {
        window->is_closed = true;
      }
      if (all_windows_closed(app)) {
        app->shutdown_started = true;
        app->run_exit_code = 0;
      }
    }
    DestroyWindow(hwnd);
    return 0;
  }

  if (message == WM_DESTROY) {
    if (auto app = self->app_state_.lock(); app && app->shutdown_started) {
      PostQuitMessage(0);
    }
    return 0;
  }

  if (message == WM_ERASEBKGND && self->borderless_) {
    return 1;
  }

  if (message == WM_PAINT && self->borderless_) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
    EndPaint(hwnd, &ps);
    return 0;
  }

  if (message == WM_SIZE && self->webview_host_) {
    auto rect = self->client_rect();
    (void)self->webview_host_->set_bounds(rect);
  }

  if (message == WM_MOUSEMOVE) {
    TRACKMOUSEEVENT track{};
    track.cbSize = sizeof(track);
    track.dwFlags = TME_LEAVE;
    track.hwndTrack = hwnd;
    TrackMouseEvent(&track);
  }

  if (message == WM_MOUSELEAVE) {
    if (self->subscribed_events_.count("host-mouseleave")) {
      Json payload{{"kind", "native_event"}, {"name", "host-mouseleave"}, {"payload", Json::object()}};
      (void)self->webview_host_->post_json_message(payload.dump());
    }
    return 0;
  }

  if (message == WM_TIMER && wparam == 1) {
    POINT pt;
    GetCursorPos(&pt);
    RECT rect;
    GetWindowRect(hwnd, &rect);
    if (!PtInRect(&rect, pt)) {
      if (self->subscribed_events_.count("host-mouseleave")) {
        Json payload{{"kind", "native_event"}, {"name", "host-mouseleave"}, {"payload", Json::object()}};
        (void)self->webview_host_->post_json_message(payload.dump());
      }
    }
    return 0;
  }

  return DefWindowProcW(hwnd, message, wparam, lparam);
}

Result<std::shared_ptr<Win32WindowHost>> Win32WindowHost::create(
    std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state,
    const WindowOptions& options) {
  auto host = std::shared_ptr<Win32WindowHost>(
      new Win32WindowHost(std::move(app_state), std::move(window_state)));

  host->size_ = {options.width, options.height};
  host->position_ = {options.x.value_or(CW_USEDEFAULT), options.y.value_or(CW_USEDEFAULT)};
  host->borderless_ = options.borderless;
  host->always_on_top_ = options.always_on_top;
  host->show_in_taskbar_ = options.show_in_taskbar;
  host->resizable_ = options.resizable;
  host->webview_host_ = std::make_unique<Win32WebviewHost>();
  host->invoke_bus_ = std::make_unique<InvokeBus>();
  host->webview_host_->set_transparent_background(options.borderless);
  host->webview_host_->set_message_handler([host_ptr = host.get()](std::string_view message) {
    auto parsed = nlohmann::json::parse(message, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
      return;
    }
    auto kind_it = parsed.find("kind");
    auto name_it = parsed.find("name");
    auto payload_it = parsed.find("payload");
    auto request_id_it = parsed.find("requestId");
    if (kind_it == parsed.end() || !kind_it->is_string()) {
      return;
    }
    std::string kind = *kind_it;
    std::string name = (name_it != parsed.end() && name_it->is_string()) ? std::string(*name_it) : std::string();
    Json payload = (payload_it != parsed.end()) ? *payload_it : Json::object();
    if (kind == "close") {
      (void)host_ptr->close();
      return;
    }
    if (kind == "drag") {
      ReleaseCapture();
      SendMessageW(host_ptr->hwnd_, WM_NCLBUTTONDOWN, HTCAPTION, 0);
      return;
    }
    if (kind == "subscribe") {
      host_ptr->subscribed_events_.insert(name);
      if (name == "host-mouseleave" && host_ptr->hwnd_) {
        SetTimer(host_ptr->hwnd_, 1, 100, nullptr);
      }
      return;
    }
    if (kind == "unsubscribe") {
      host_ptr->subscribed_events_.erase(name);
      if (name == "host-mouseleave" && host_ptr->hwnd_) {
        KillTimer(host_ptr->hwnd_, 1);
      }
      return;
    }
    if (kind == "invoke") {
      auto result = host_ptr->invoke_bus_->dispatch(name, payload);
      Json message_out{{"kind", "invoke_result"}, {"name", name}, {"ok", static_cast<bool>(result)}, {"payload", result ? *result : Json::object()}};
      if (request_id_it != parsed.end() && request_id_it->is_number_unsigned()) {
        message_out["requestId"] = *request_id_it;
      }
      if (!result) {
        message_out["error"] = Json{{"code", result.error().code}, {"message", result.error().message}};
      }
      (void)host_ptr->webview_host_->post_json_message(message_out.dump());
      return;
    }
    if (kind == "emit") {
      if (host_ptr->subscribed_events_.count(name)) {
        Json message_out{{"kind", "native_event"}, {"name", name}, {"payload", payload}};
        (void)host_ptr->webview_host_->post_json_message(message_out.dump());
      }
    }
  });

  const wchar_t* class_name = L"ViewshellWindow";
  WNDCLASSW wc{};
  wc.lpfnWndProc = Win32WindowHost::WindowProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = class_name;
  wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));
  RegisterClassW(&wc);

  DWORD style = host->borderless_ ? WS_POPUP : WS_OVERLAPPEDWINDOW;
  if (!host->resizable_) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  }

  DWORD ex_style = 0;
  if (host->always_on_top_) {
    ex_style |= WS_EX_TOPMOST;
  }
  if (!host->show_in_taskbar_) {
    ex_style |= WS_EX_TOOLWINDOW;
  } else {
    ex_style |= WS_EX_APPWINDOW;
  }

  host->hwnd_ = CreateWindowExW(
      ex_style,
      class_name,
      L"Viewshell",
      style,
      host->position_.x,
      host->position_.y,
      host->size_.width,
      host->size_.height,
      nullptr,
      nullptr,
      GetModuleHandleW(nullptr),
      host.get());

  if (!host->hwnd_) {
    return tl::unexpected(Error{"window_creation_failed", "failed to create Win32 window"});
  }

  ShowWindow(host->hwnd_, SW_SHOW);
  UpdateWindow(host->hwnd_);

  if (host->borderless_) {
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(host->hwnd_, &margins);
  }

  auto attach_result = host->webview_host_->attach(host->hwnd_, options);
  if (!attach_result) {
    return tl::unexpected(attach_result.error());
  }

  if (options.borderless) {
    (void)host->webview_host_->add_init_script(kWindowsBridgeBootstrap);
  } else {
    (void)host->webview_host_->add_init_script(kWindowsBridgeBootstrap);
  }

  if (options.asset_root.has_value() && !options.asset_root->empty()) {
    auto load_result = host->webview_host_->load_file(*options.asset_root);
    if (!load_result) {
      return tl::unexpected(load_result.error());
    }
  }

  return host;
}

void Win32WindowHost::run_message_loop() {
  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

Result<void> Win32WindowHost::ensure_window() const {
  if (!hwnd_) {
    return tl::unexpected(Error{"invalid_state", "window is not available"});
  }
  return {};
}

void Win32WindowHost::update_style() {
  if (!hwnd_) {
    return;
  }

  LONG_PTR style = borderless_ ? WS_POPUP : WS_OVERLAPPEDWINDOW;
  if (!resizable_) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  }
  SetWindowLongPtrW(hwnd_, GWL_STYLE, style);
  LONG_PTR ex_style = always_on_top_ ? WS_EX_TOPMOST : 0;
  if (!show_in_taskbar_) {
    ex_style |= WS_EX_TOOLWINDOW;
  } else {
    ex_style |= WS_EX_APPWINDOW;
  }
  SetWindowLongPtrW(hwnd_, GWL_EXSTYLE, ex_style);
  SetWindowPos(hwnd_, always_on_top_ ? HWND_TOPMOST : HWND_NOTOPMOST,
      0, 0, 0, 0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

RECT Win32WindowHost::client_rect() const {
  RECT rect{};
  if (hwnd_) {
    GetClientRect(hwnd_, &rect);
  }
  return rect;
}

Result<void> Win32WindowHost::set_title(std::string_view title) {
  if (auto result = ensure_window(); !result) return result;
  auto wide = to_wstring(title);
  SetWindowTextW(hwnd_, wide.c_str());
  return {};
}

Result<void> Win32WindowHost::maximize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_MAXIMIZE);
  return {};
}

Result<void> Win32WindowHost::unmaximize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_RESTORE);
  return {};
}

Result<void> Win32WindowHost::minimize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_MINIMIZE);
  return {};
}

Result<void> Win32WindowHost::unminimize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_RESTORE);
  return {};
}

Result<void> Win32WindowHost::show() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_SHOW);
  return {};
}

Result<void> Win32WindowHost::hide() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_HIDE);
  return {};
}

Result<void> Win32WindowHost::focus() {
  if (auto result = ensure_window(); !result) return result;
  SetForegroundWindow(hwnd_);
  SetFocus(hwnd_);
  return {};
}

Result<void> Win32WindowHost::set_geometry(Geometry geometry) {
  if (auto result = ensure_window(); !result) return result;
  position_ = {geometry.x, geometry.y};
  size_ = {geometry.width, geometry.height};
  MoveWindow(hwnd_, geometry.x, geometry.y, geometry.width, geometry.height, TRUE);
  if (webview_host_) {
    (void)webview_host_->set_bounds(client_rect());
  }
  RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
  return {};
}

Result<Geometry> Win32WindowHost::get_geometry() const {
  if (auto result = ensure_window(); !result) return tl::unexpected(result.error());
  RECT rect{};
  GetWindowRect(hwnd_, &rect);
  return Geometry{rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top};
}

Result<void> Win32WindowHost::set_size(Size size) {
  if (auto result = ensure_window(); !result) return result;
  size_ = size;
  SetWindowPos(hwnd_, nullptr, 0, 0, size.width, size.height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  if (webview_host_) {
    (void)webview_host_->set_bounds(client_rect());
  }
  InvalidateRect(hwnd_, nullptr, TRUE);
  return {};
}

Result<Size> Win32WindowHost::get_size() const {
  if (auto result = ensure_window(); !result) return tl::unexpected(result.error());
  RECT rect{};
  GetWindowRect(hwnd_, &rect);
  return Size{rect.right - rect.left, rect.bottom - rect.top};
}

Result<void> Win32WindowHost::set_position(Position pos) {
  if (auto result = ensure_window(); !result) return result;
  position_ = pos;
  SetWindowPos(hwnd_, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  if (webview_host_) {
    (void)webview_host_->set_bounds(client_rect());
  }
  return {};
}

Result<Position> Win32WindowHost::get_position() const {
  if (auto result = ensure_window(); !result) return tl::unexpected(result.error());
  RECT rect{};
  GetWindowRect(hwnd_, &rect);
  return Position{rect.left, rect.top};
}

Result<void> Win32WindowHost::set_borderless(bool enabled) {
  borderless_ = enabled;
  update_style();
  return {};
}

Result<void> Win32WindowHost::set_always_on_top(bool enabled) {
  always_on_top_ = enabled;
  update_style();
  return {};
}

Result<void> Win32WindowHost::close() {
  if (auto result = ensure_window(); !result) return result;
  SendMessageW(hwnd_, WM_CLOSE, 0, 0);
  return {};
}

Result<void> Win32WindowHost::load_url(std::string_view url) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->load_url(url);
}
Result<void> Win32WindowHost::load_file(std::string_view path) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->load_file(path);
}
Result<void> Win32WindowHost::reload() { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::evaluate_script(std::string_view script) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->evaluate_script(script);
}
Result<void> Win32WindowHost::add_init_script(std::string_view script) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->add_init_script(script);
}
Result<void> Win32WindowHost::open_devtools() {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->open_devtools();
}
Result<void> Win32WindowHost::close_devtools() {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->close_devtools();
}
Result<void> Win32WindowHost::on_page_load(PageLoadHandler handler) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->on_page_load(std::move(handler));
}
Result<void> Win32WindowHost::set_navigation_handler(NavigationHandler handler) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->set_navigation_handler(std::move(handler));
}
Result<void> Win32WindowHost::register_command(std::string name, CommandHandler handler) {
  return invoke_bus_->register_command(std::move(name), std::move(handler));
}
Result<void> Win32WindowHost::emit(std::string name, const Json& payload) {
  if (!subscribed_events_.count(name)) {
    return {};
  }
  Json message{{"kind", "native_event"}, {"name", name}, {"payload", payload}};
  return webview_host_->post_json_message(message.dump());
}

Result<Capabilities> Win32WindowHost::capabilities() const {
  Capabilities caps;
  caps.window.borderless = true;
  caps.window.transparent = true;
  caps.window.always_on_top = true;
  caps.window.native_drag = true;
  caps.webview.devtools = true;
  caps.webview.script_eval = true;
  caps.bridge.invoke = true;
  caps.bridge.native_events = true;
  return caps;
}

} // namespace viewshell

#endif
