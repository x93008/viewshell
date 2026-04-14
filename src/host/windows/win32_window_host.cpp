#ifdef _WIN32

#include "win32_window_host.h"

#include <string>

#include "webview/win32_webview_host.h"
#include "viewshell/runtime_state.h"

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

Error unsupported_webview_error() {
  return Error{"unsupported_by_backend", "windows webview backend not implemented yet"};
}

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
      app->shutdown_started = true;
      app->run_exit_code = 0;
    }
    if (auto window = self->window_state_.lock()) {
      window->is_closed = true;
    }
    DestroyWindow(hwnd);
    return 0;
  }

  if (message == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }

  if (message == WM_SIZE && self->webview_host_) {
    auto rect = self->client_rect();
    (void)self->webview_host_->set_bounds(rect);
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
  host->webview_host_ = std::make_unique<Win32WebviewHost>();

  const wchar_t* class_name = L"ViewshellWindow";
  WNDCLASSW wc{};
  wc.lpfnWndProc = Win32WindowHost::WindowProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = class_name;
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  RegisterClassW(&wc);

  DWORD style = WS_OVERLAPPEDWINDOW;
  if (host->borderless_) {
    style = WS_POPUP | WS_THICKFRAME;
  }

  host->hwnd_ = CreateWindowExW(
      host->always_on_top_ ? WS_EX_TOPMOST : 0,
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

  auto attach_result = host->webview_host_->attach(host->hwnd_, options);
  if (!attach_result) {
    return tl::unexpected(attach_result.error());
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

  LONG_PTR style = borderless_ ? (WS_POPUP | WS_THICKFRAME) : WS_OVERLAPPEDWINDOW;
  SetWindowLongPtrW(hwnd_, GWL_STYLE, style);
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

Result<void> Win32WindowHost::set_size(Size size) {
  if (auto result = ensure_window(); !result) return result;
  size_ = size;
  SetWindowPos(hwnd_, nullptr, 0, 0, size.width, size.height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
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
Result<void> Win32WindowHost::load_file(std::string_view) { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::reload() { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::evaluate_script(std::string_view script) {
  if (!webview_host_) return tl::unexpected(unsupported_webview_error());
  return webview_host_->evaluate_script(script);
}
Result<void> Win32WindowHost::add_init_script(std::string_view) { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::open_devtools() { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::close_devtools() { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::on_page_load(PageLoadHandler) { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::set_navigation_handler(NavigationHandler) { return tl::unexpected(unsupported_webview_error()); }
Result<void> Win32WindowHost::register_command(std::string, CommandHandler) { return tl::unexpected(Error{"unsupported_by_backend", "windows bridge backend not implemented yet"}); }
Result<void> Win32WindowHost::emit(std::string, const Json&) { return tl::unexpected(Error{"bridge_unavailable", "windows bridge backend not implemented yet"}); }

Result<Capabilities> Win32WindowHost::capabilities() const {
  Capabilities caps;
  caps.window.borderless = true;
  caps.window.always_on_top = true;
  return caps;
}

} // namespace viewshell

#endif
