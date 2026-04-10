#pragma once

#include <viewshell/types.h>

namespace viewshell {

struct Capabilities {
  struct Window {
    bool borderless = false;
    bool transparent = false;
    bool always_on_top = false;
    bool native_drag = false;
  } window;

  struct Webview {
    bool devtools = false;
    bool resource_protocol = false;
    bool script_eval = false;
  } webview;

  struct Bridge {
    bool invoke = false;
    bool native_events = false;
  } bridge;
};

} // namespace viewshell
