#pragma once

#include <string>
#include <vector>
#include <functional>

namespace viewshell {

struct TrayMenuItem {
  std::string id;
  std::string label;  // empty = separator
  bool enabled = true;
};

struct TrayOptions {
  std::string icon_path;
  std::string tooltip;
  std::vector<TrayMenuItem> menu;
  std::function<void()> on_click;
  std::function<void(const std::string& id)> on_menu_click;
};

} // namespace viewshell
