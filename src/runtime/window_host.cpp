#include "runtime/window_host.h"

#include <nlohmann/json.hpp>

namespace viewshell {

void WindowHost::apply_common_options(const WindowOptions& options) {
  borderless_ = options.borderless;
  always_on_top_ = options.always_on_top;
  show_in_taskbar_ = options.show_in_taskbar;
  resizable_ = options.resizable;
  inject_window_api_ = options.inject_window_api;
}

bool WindowHost::handle_wnd_command(const std::string& name, const Json& payload, Json& out_payload, Result<void>& out_result) {
  if (!inject_window_api_ || name.rfind("__wnd.", 0) != 0) {
    return false;
  }

  out_result = tl::unexpected(Error{"unknown_command", "unknown __wnd command: " + name});

  if (name == "__wnd.startDrag") {
    begin_drag();
    out_result = {};
    out_payload = Json{{"ok", true}};
  } else if (name == "__wnd.setPosition") {
    int px = payload.value("x", 0);
    int py = payload.value("y", 0);
    out_result = set_position({px, py});
    out_payload = Json{{"ok", true}};
  } else if (name == "__wnd.setSize") {
    int sw = payload.value("width", 0);
    int sh = payload.value("height", 0);
    out_result = set_size({sw, sh});
    out_payload = Json{{"ok", true}};
  } else if (name == "__wnd.setGeometry") {
    int gx = payload.value("x", 0);
    int gy = payload.value("y", 0);
    int gw = payload.value("width", 0);
    int gh = payload.value("height", 0);
    out_result = set_geometry({gx, gy, gw, gh});
    out_payload = Json{{"ok", true}};
  } else if (name == "__wnd.getGeometry") {
    auto geo = get_geometry();
    if (geo) {
      out_result = {};
      out_payload = Json{{"x", geo->x}, {"y", geo->y}, {"width", geo->width}, {"height", geo->height}};
    } else {
      out_result = tl::unexpected(geo.error());
    }
  } else if (name == "__wnd.close") {
    out_result = close();
    out_payload = Json{{"ok", true}};
  }

  return true;
}

} // namespace viewshell
