#pragma once

#include <gtk/gtk.h>
#include <X11/Xlib.h>

namespace viewshell {

struct NativeWindowHandle {
  GtkWidget* gtk_window = nullptr;
  Display* x11_display = nullptr;
  unsigned long x11_window = 0;
};

}
