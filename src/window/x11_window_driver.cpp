#include "window/x11_window_driver.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

namespace viewshell {

static void on_gtk_destroy(GtkWidget*, gpointer user_data) {
  auto* driver = static_cast<WindowDriver*>(user_data);
  if (driver && driver->on_close) driver->on_close();
}

static gboolean on_gtk_configure(GtkWidget*, GdkEventConfigure* event, gpointer user_data) {
  auto* driver = static_cast<WindowDriver*>(user_data);
  if (driver && driver->on_resize) driver->on_resize(Size{event->width, event->height});
  return FALSE;
}

static gboolean on_gtk_focus(GtkWidget*, GdkEventFocus* event, gpointer user_data) {
  auto* driver = static_cast<WindowDriver*>(user_data);
  if (driver && driver->on_focus) driver->on_focus(event->in != 0);
  return FALSE;
}

static gboolean on_gtk_button_press(GtkWidget* widget, GdkEventButton* event, gpointer) {
  if (event->button == 1) {
    gtk_window_begin_move_drag(GTK_WINDOW(widget), event->button,
        (gint)event->x_root, (gint)event->y_root, event->time);
  }
  return FALSE;
}

WindowDriver::~WindowDriver() {
  on_close = nullptr;
  if (created_ && gtk_window_) {
    gtk_widget_destroy(gtk_window_);
    gtk_window_ = nullptr;
    created_ = false;
  }
}

Result<NativeWindowHandle> WindowDriver::create(const WindowOptions& options) {
  if (!gtk_init_check(nullptr, nullptr)) {
    return tl::unexpected(Error{"engine_init_failed", "gtk_init_check failed"});
  }

  gtk_window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  if (!gtk_window_) {
    return tl::unexpected(Error{"engine_init_failed", "failed to create GTK window"});
  }

  gtk_window_set_default_size(GTK_WINDOW(gtk_window_), options.width, options.height);
  gtk_window_set_title(GTK_WINDOW(gtk_window_), "Viewshell");

  if (options.borderless) {
    gtk_window_set_decorated(GTK_WINDOW(gtk_window_), FALSE);
    gtk_widget_set_app_paintable(gtk_window_, TRUE);

    auto screen = gtk_widget_get_screen(gtk_window_);
    auto visual = gdk_screen_get_rgba_visual(screen);
    if (visual) gtk_widget_set_visual(gtk_window_, visual);

    gtk_widget_add_events(gtk_window_, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(gtk_window_, "button-press-event",
        G_CALLBACK(on_gtk_button_press), nullptr);
  }

  if (options.always_on_top) {
    gtk_window_set_keep_above(GTK_WINDOW(gtk_window_), TRUE);
  }

  if (!options.show_in_taskbar) {
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(gtk_window_), TRUE);
  }

  g_signal_connect(gtk_window_, "destroy", G_CALLBACK(on_gtk_destroy), this);
  g_signal_connect(gtk_window_, "configure-event", G_CALLBACK(on_gtk_configure), this);
  g_signal_connect(gtk_window_, "focus-in-event", G_CALLBACK(on_gtk_focus), this);
  g_signal_connect(gtk_window_, "focus-out-event", G_CALLBACK(on_gtk_focus), this);

  if (options.x && options.y) {
    gtk_window_move(GTK_WINDOW(gtk_window_), *options.x, *options.y);
  }

  created_ = true;

  NativeWindowHandle handle;
  handle.gtk_window = gtk_window_;
  GdkWindow* gdk_win = gtk_widget_get_window(gtk_window_);
  if (gdk_win) {
    handle.x11_display = GDK_DISPLAY_XDISPLAY(gdk_window_get_display(gdk_win));
    handle.x11_window = GDK_WINDOW_XID(gdk_win);
  }
  return handle;
}

Result<void> WindowDriver::set_title(std::string_view title) {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_set_title(GTK_WINDOW(gtk_window_), std::string(title).c_str());
  return {};
}

Result<void> WindowDriver::set_size(Size size) {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_resize(GTK_WINDOW(gtk_window_), size.width, size.height);
  return {};
}

Result<Size> WindowDriver::get_size() const {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  int w, h;
  gtk_window_get_size(GTK_WINDOW(gtk_window_), &w, &h);
  return Size{w, h};
}

Result<void> WindowDriver::set_position(Position pos) {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  pending_position_ = pos;
  has_pending_position_ = true;
  gtk_window_move(GTK_WINDOW(gtk_window_), pos.x, pos.y);
  return {};
}

Result<Position> WindowDriver::get_position() const {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  int x, y;
  gtk_window_get_position(GTK_WINDOW(gtk_window_), &x, &y);
  return Position{x, y};
}

Result<void> WindowDriver::set_borderless(bool enabled) {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_set_decorated(GTK_WINDOW(gtk_window_), !enabled);
  return {};
}

Result<void> WindowDriver::set_always_on_top(bool enabled) {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_set_keep_above(GTK_WINDOW(gtk_window_), enabled);
  return {};
}

Result<void> WindowDriver::maximize() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_maximize(GTK_WINDOW(gtk_window_));
  return {};
}

Result<void> WindowDriver::unmaximize() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_unmaximize(GTK_WINDOW(gtk_window_));
  return {};
}

Result<void> WindowDriver::minimize() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_iconify(GTK_WINDOW(gtk_window_));
  return {};
}

Result<void> WindowDriver::unminimize() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_deiconify(GTK_WINDOW(gtk_window_));
  return {};
}

Result<void> WindowDriver::show() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_widget_show_all(gtk_window_);
  // Re-apply position after show, because WM may ignore position set on hidden windows
  if (has_pending_position_) {
    gtk_window_move(GTK_WINDOW(gtk_window_), pending_position_.x, pending_position_.y);
    has_pending_position_ = false;
  }
  return {};
}

Result<void> WindowDriver::hide() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_widget_hide(gtk_window_);
  return {};
}

Result<void> WindowDriver::focus() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_window_present(GTK_WINDOW(gtk_window_));
  return {};
}

Result<void> WindowDriver::begin_drag(DragContext ctx) {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  if (!ctx.is_valid) {
    return tl::unexpected(Error{"invalid_drag_context", "no active drag context"});
  }
  GdkWindow* gdk_win = gtk_widget_get_window(gtk_window_);
  if (!gdk_win) return tl::unexpected(Error{"window_not_ready", ""});
  Display* dpy = GDK_DISPLAY_XDISPLAY(gdk_window_get_display(gdk_win));
  XUngrabPointer(dpy, ctx.timestamp);
  XFlush(dpy);
  gtk_window_begin_move_drag(GTK_WINDOW(gtk_window_), ctx.button, ctx.root_x, ctx.root_y, ctx.timestamp);
  return {};
}

Result<void> WindowDriver::close() {
  if (!created_) return tl::unexpected(Error{"window_not_ready", ""});
  gtk_widget_destroy(gtk_window_);
  gtk_window_ = nullptr;
  created_ = false;
  return {};
}

void WindowDriver::run_main_loop() {
  if (created_) gtk_main();
}

void WindowDriver::quit_main_loop() {
  gtk_main_quit();
}

}
