#pragma once

#include <string>
#include <optional>
#include <vector>
#include <viewshell/types.h>

namespace viewshell {

struct AppOptions {
  int bridge_timeout_ms = 5000;
  std::vector<std::string> trusted_origins;
  std::optional<std::string> require_engine;
  std::optional<std::string> engine_path;
};

struct WindowOptions {
  std::optional<std::string> asset_root;
  int width = 800;
  int height = 600;
  std::optional<int> x;
  std::optional<int> y;
  bool borderless = false;
  bool always_on_top = false;
  bool show_in_taskbar = true;
  bool resizable = true;
  bool inject_window_api = false;
  bool dismiss_on_outside_click = false;  // X11 only: hide window when clicking outside
};

struct NormalizedAppOptions {
  int bridge_timeout_ms;
  std::vector<std::string> trusted_origins;
  std::optional<std::string> require_engine;
  std::optional<std::string> engine_path;
};

} // namespace viewshell
