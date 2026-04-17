#pragma once

#ifdef _WIN32

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <shellapi.h>

#include <viewshell/tray_options.h>
#include <viewshell/types.h>

#include "runtime/tray_host.h"

namespace viewshell {

class Win32TrayHost final : public TrayHost {
public:
  static Result<std::shared_ptr<Win32TrayHost>> create(const TrayOptions& options);

  ~Win32TrayHost() override;

  Result<void> set_icon(std::string_view icon_path) override;
  Result<void> set_tooltip(std::string_view tooltip) override;
  Result<void> set_menu(std::vector<TrayMenuItem> menu) override;
  Result<void> remove() override;

private:
  Win32TrayHost();

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  void rebuild_menu();
  Result<void> load_icon(std::string_view icon_path);

  HWND hwnd_ = nullptr;
  NOTIFYICONDATAW nid_ = {};
  HMENU hmenu_ = nullptr;
  HICON hicon_ = nullptr;

  std::vector<TrayMenuItem> menu_items_;
  std::function<void()> on_click_;
  std::function<void(const std::string& id)> on_menu_click_;
};

} // namespace viewshell

#endif
