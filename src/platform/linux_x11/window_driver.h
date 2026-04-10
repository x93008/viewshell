#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <viewshell/types.h>
#include <viewshell/options.h>
#include "native_window_handle.h"
#include "drag_context.h"

namespace viewshell {

class WindowDriver {
public:
  Result<NativeWindowHandle> create(const WindowOptions& options);
  Result<void> set_title(std::string_view title);
  Result<void> set_size(Size size);
  Result<Size> get_size() const;
  Result<void> set_position(Position position);
  Result<Position> get_position() const;
  Result<void> set_borderless(bool enabled);
  Result<void> set_always_on_top(bool enabled);
  Result<void> maximize();
  Result<void> unmaximize();
  Result<void> minimize();
  Result<void> unminimize();
  Result<void> show();
  Result<void> hide();
  Result<void> focus();
  Result<void> begin_drag(DragContext ctx);
  Result<void> close();

  std::function<void()> on_close;
  std::function<void(Size)> on_resize;
  std::function<void(bool)> on_focus;

private:
  GtkWidget* gtk_window_ = nullptr;
  bool created_ = false;
};

}
