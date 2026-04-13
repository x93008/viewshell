#ifdef _WIN32

#include "windows_window_host.h"

#include <string>

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

WindowsWindowHost::WindowsWindowHost(std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state)
    : app_state_(std::move(app_state)),
      window_state_(std::move(window_state)) {}

WindowsWindowHost::~WindowsWindowHost() {
  if (hwnd_) {
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}

LRESULT CALLBACK WindowsWindowHost::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_NCCREATE) {
    auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
    auto* self = static_cast<WindowsWindowHost*>(create->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    return DefWindowProcW(hwnd, message, wparam, lparam);
  }

  auto* self = reinterpret_cast<WindowsWindowHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
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

  return DefWindowProcW(hwnd, message, wparam, lparam);
}

Result<std::shared_ptr<WindowsWindowHost>> WindowsWindowHost::create(
    std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state,
    const WindowOptions& options) {
  auto host = std::shared_ptr<WindowsWindowHost>(
      new WindowsWindowHost(std::move(app_state), std::move(window_state)));

  host->size_ = {options.width, options.height};
  host->position_ = {options.x.value_or(CW_USEDEFAULT), options.y.value_or(CW_USEDEFAULT)};
  host->borderless_ = options.borderless;
  host->always_on_top_ = options.always_on_top;

  const wchar_t* class_name = L"ViewshellWindow";
  WNDCLASSW wc{};
  wc.lpfnWndProc = WindowsWindowHost::WindowProc;
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
  return host;
}

void WindowsWindowHost::run_message_loop() {
  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

Result<void> WindowsWindowHost::ensure_window() const {
  if (!hwnd_) {
    return tl::unexpected(Error{"invalid_state", "window is not available"});
  }
  return {};
}

void WindowsWindowHost::update_style() {
  if (!hwnd_) {
    return;
  }

  LONG_PTR style = borderless_ ? (WS_POPUP | WS_THICKFRAME) : WS_OVERLAPPEDWINDOW;
  SetWindowLongPtrW(hwnd_, GWL_STYLE, style);
  SetWindowPos(hwnd_, always_on_top_ ? HWND_TOPMOST : HWND_NOTOPMOST,
      0, 0, 0, 0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

Result<void> WindowsWindowHost::set_title(std::string_view title) {
  if (auto result = ensure_window(); !result) return result;
  auto wide = to_wstring(title);
  SetWindowTextW(hwnd_, wide.c_str());
  return {};
}

Result<void> WindowsWindowHost::maximize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_MAXIMIZE);
  return {};
}

Result<void> WindowsWindowHost::unmaximize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_RESTORE);
  return {};
}

Result<void> WindowsWindowHost::minimize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_MINIMIZE);
  return {};
}

Result<void> WindowsWindowHost::unminimize() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_RESTORE);
  return {};
}

Result<void> WindowsWindowHost::show() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_SHOW);
  return {};
}

Result<void> WindowsWindowHost::hide() {
  if (auto result = ensure_window(); !result) return result;
  ShowWindow(hwnd_, SW_HIDE);
  return {};
}

Result<void> WindowsWindowHost::focus() {
  if (auto result = ensure_window(); !result) return result;
  SetForegroundWindow(hwnd_);
  SetFocus(hwnd_);
  return {};
}

Result<void> WindowsWindowHost::set_size(Size size) {
  if (auto result = ensure_window(); !result) return result;
  size_ = size;
  SetWindowPos(hwnd_, nullptr, 0, 0, size.width, size.height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  return {};
}

Result<Size> WindowsWindowHost::get_size() const {
  if (auto result = ensure_window(); !result) return tl::unexpected(result.error());
  RECT rect{};
  GetWindowRect(hwnd_, &rect);
  return Size{rect.right - rect.left, rect.bottom - rect.top};
}

Result<void> WindowsWindowHost::set_position(Position pos) {
  if (auto result = ensure_window(); !result) return result;
  position_ = pos;
  SetWindowPos(hwnd_, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  return {};
}

Result<Position> WindowsWindowHost::get_position() const {
  if (auto result = ensure_window(); !result) return tl::unexpected(result.error());
  RECT rect{};
  GetWindowRect(hwnd_, &rect);
  return Position{rect.left, rect.top};
}

Result<void> WindowsWindowHost::set_borderless(bool enabled) {
  borderless_ = enabled;
  update_style();
  return {};
}

Result<void> WindowsWindowHost::set_always_on_top(bool enabled) {
  always_on_top_ = enabled;
  update_style();
  return {};
}

Result<void> WindowsWindowHost::close() {
  if (auto result = ensure_window(); !result) return result;
  SendMessageW(hwnd_, WM_CLOSE, 0, 0);
  return {};
}

Result<void> WindowsWindowHost::load_url(std::string_view) { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::load_file(std::string_view) { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::reload() { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::evaluate_script(std::string_view) { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::add_init_script(std::string_view) { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::open_devtools() { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::close_devtools() { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::on_page_load(PageLoadHandler) { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::set_navigation_handler(NavigationHandler) { return tl::unexpected(unsupported_webview_error()); }
Result<void> WindowsWindowHost::register_command(std::string, CommandHandler) { return tl::unexpected(Error{"unsupported_by_backend", "windows bridge backend not implemented yet"}); }
Result<void> WindowsWindowHost::emit(std::string, const Json&) { return tl::unexpected(Error{"bridge_unavailable", "windows bridge backend not implemented yet"}); }

Result<Capabilities> WindowsWindowHost::capabilities() const {
  Capabilities caps;
  caps.window.borderless = true;
  caps.window.always_on_top = true;
  return caps;
}

} // namespace viewshell

#endif
