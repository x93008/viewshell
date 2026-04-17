#ifdef _WIN32

#include "win32_tray_host.h"

#include <string>
#include <objidl.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace viewshell {

namespace {

constexpr UINT WM_TRAY_CALLBACK = WM_APP + 1;
constexpr UINT MENU_ID_BASE = 1000;

static ULONG_PTR gdiplus_token_ = 0;

void ensure_gdiplus() {
  if (gdiplus_token_ == 0) {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&gdiplus_token_, &input, nullptr);
  }
}

std::wstring to_wstring(std::string_view value) {
  if (value.empty()) {
    return {};
  }
  int size = MultiByteToWideChar(CP_UTF8, 0, value.data(),
      static_cast<int>(value.size()), nullptr, 0);
  std::wstring out(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(),
      static_cast<int>(value.size()), out.data(), size);
  return out;
}

} // namespace

Win32TrayHost::Win32TrayHost() = default;

Win32TrayHost::~Win32TrayHost() {
  remove();
}

LRESULT CALLBACK Win32TrayHost::WindowProc(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* self = reinterpret_cast<Win32TrayHost*>(
      GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  if (message == WM_TRAY_CALLBACK && self) {
    UINT tray_event = LOWORD(lparam);
    if (tray_event == WM_LBUTTONUP) {
      if (self->on_click_) {
        self->on_click_();
      }
    } else if (tray_event == WM_RBUTTONUP) {
      if (self->hmenu_) {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(self->hmenu_, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
            pt.x, pt.y, 0, hwnd, nullptr);
        PostMessage(hwnd, WM_NULL, 0, 0);
      }
    }
    return 0;
  }

  if (message == WM_COMMAND && self) {
    UINT cmd_id = LOWORD(wparam);
    if (cmd_id >= MENU_ID_BASE) {
      UINT index = cmd_id - MENU_ID_BASE;
      if (index < self->menu_items_.size() && self->on_menu_click_) {
        self->on_menu_click_(self->menu_items_[index].id);
      }
    }
    return 0;
  }

  return DefWindowProcW(hwnd, message, wparam, lparam);
}

Result<std::shared_ptr<Win32TrayHost>> Win32TrayHost::create(
    const TrayOptions& options) {
  auto host = std::shared_ptr<Win32TrayHost>(new Win32TrayHost());

  host->on_click_ = options.on_click;
  host->on_menu_click_ = options.on_menu_click;
  host->menu_items_ = options.menu;

  // Register window class
  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"ViewShellTrayHost";
  RegisterClassExW(&wc);

  // Create hidden message-only window
  host->hwnd_ = CreateWindowExW(0, L"ViewShellTrayHost", L"",
      0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
      GetModuleHandleW(nullptr), nullptr);

  if (!host->hwnd_) {
    return tl::unexpected(Error{"tray_create_failed",
        "Failed to create message-only window for tray icon"});
  }

  SetWindowLongPtrW(host->hwnd_, GWLP_USERDATA,
      reinterpret_cast<LONG_PTR>(host.get()));

  // Initialize NOTIFYICONDATAW
  host->nid_ = {};
  host->nid_.cbSize = sizeof(NOTIFYICONDATAW);
  host->nid_.hWnd = host->hwnd_;
  host->nid_.uID = 1;
  host->nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  host->nid_.uCallbackMessage = WM_TRAY_CALLBACK;
  host->nid_.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));

  // Load custom icon if provided
  if (!options.icon_path.empty()) {
    auto result = host->load_icon(options.icon_path);
    if (!result) {
      return tl::unexpected(result.error());
    }
  }

  // Set tooltip
  if (!options.tooltip.empty()) {
    auto wide = to_wstring(options.tooltip);
    wcsncpy_s(host->nid_.szTip, wide.c_str(), _TRUNCATE);
  }

  if (!Shell_NotifyIconW(NIM_ADD, &host->nid_)) {
    DestroyWindow(host->hwnd_);
    host->hwnd_ = nullptr;
    return tl::unexpected(Error{"tray_create_failed",
        "Shell_NotifyIconW NIM_ADD failed"});
  }

  // Build context menu
  if (!options.menu.empty()) {
    host->rebuild_menu();
  }

  return host;
}

Result<void> Win32TrayHost::load_icon(std::string_view icon_path) {
  auto wide_path = to_wstring(icon_path);
  HICON icon = nullptr;

  // Try LoadImageW first (supports .ico/.bmp)
  icon = static_cast<HICON>(LoadImageW(nullptr, wide_path.c_str(),
      IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));

  // Fallback to GDI+ for .png and other formats
  if (!icon) {
    ensure_gdiplus();
    auto* bitmap = Gdiplus::Bitmap::FromFile(wide_path.c_str());
    if (bitmap && bitmap->GetLastStatus() == Gdiplus::Ok) {
      bitmap->GetHICON(&icon);
      delete bitmap;
    }
  }

  if (!icon) {
    return tl::unexpected(Error{"icon_load_failed",
        "Failed to load icon from: " + std::string(icon_path)});
  }

  if (hicon_) {
    DestroyIcon(hicon_);
  }
  hicon_ = icon;
  nid_.hIcon = hicon_;
  return {};
}

Result<void> Win32TrayHost::set_icon(std::string_view icon_path) {
  auto result = load_icon(icon_path);
  if (!result) {
    return result;
  }
  if (!Shell_NotifyIconW(NIM_MODIFY, &nid_)) {
    return tl::unexpected(Error{"tray_modify_failed",
        "Shell_NotifyIconW NIM_MODIFY failed for icon update"});
  }
  return {};
}

Result<void> Win32TrayHost::set_tooltip(std::string_view tooltip) {
  auto wide = to_wstring(tooltip);
  wcsncpy_s(nid_.szTip, wide.c_str(), _TRUNCATE);
  if (!Shell_NotifyIconW(NIM_MODIFY, &nid_)) {
    return tl::unexpected(Error{"tray_modify_failed",
        "Shell_NotifyIconW NIM_MODIFY failed for tooltip update"});
  }
  return {};
}

Result<void> Win32TrayHost::set_menu(std::vector<TrayMenuItem> menu) {
  menu_items_ = std::move(menu);
  rebuild_menu();
  return {};
}

void Win32TrayHost::rebuild_menu() {
  if (hmenu_) {
    DestroyMenu(hmenu_);
    hmenu_ = nullptr;
  }

  hmenu_ = CreatePopupMenu();
  if (!hmenu_) {
    return;
  }

  for (size_t i = 0; i < menu_items_.size(); ++i) {
    const auto& item = menu_items_[i];
    if (item.label.empty()) {
      AppendMenuW(hmenu_, MF_SEPARATOR, 0, nullptr);
    } else {
      auto wide_label = to_wstring(item.label);
      UINT flags = MF_STRING;
      if (!item.enabled) {
        flags |= MF_GRAYED;
      }
      AppendMenuW(hmenu_, flags,
          static_cast<UINT_PTR>(MENU_ID_BASE + i), wide_label.c_str());
    }
  }
}

Result<Geometry> Win32TrayHost::get_icon_rect() const {
  NOTIFYICONIDENTIFIER nii = {};
  nii.cbSize = sizeof(nii);
  nii.hWnd = hwnd_;
  nii.uID = 1;

  RECT rect = {};
  HRESULT hr = Shell_NotifyIconGetRect(&nii, &rect);
  if (FAILED(hr)) {
    return tl::unexpected(Error{"icon_rect_failed",
        "Shell_NotifyIconGetRect failed"});
  }

  return Geometry{
      static_cast<int>(rect.left),
      static_cast<int>(rect.top),
      static_cast<int>(rect.right - rect.left),
      static_cast<int>(rect.bottom - rect.top)};
}

Result<void> Win32TrayHost::remove() {
  if (nid_.hWnd) {
    Shell_NotifyIconW(NIM_DELETE, &nid_);
    nid_.hWnd = nullptr;
  }
  if (hmenu_) {
    DestroyMenu(hmenu_);
    hmenu_ = nullptr;
  }
  if (hicon_) {
    DestroyIcon(hicon_);
    hicon_ = nullptr;
  }
  if (hwnd_) {
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
  return {};
}

} // namespace viewshell

#endif
