#include "x11_tray_host.h"

#include <gtk/gtk.h>

namespace viewshell {

X11TrayHost::X11TrayHost() = default;

X11TrayHost::~X11TrayHost() {
  remove();
}

Result<std::shared_ptr<X11TrayHost>> X11TrayHost::create(
    const TrayOptions& options) {
  auto host = std::shared_ptr<X11TrayHost>(new X11TrayHost());

  host->on_click_ = options.on_click;
  host->on_right_click_ = options.on_right_click;
  host->on_menu_click_ = options.on_menu_click;

  host->icon_ = gtk_status_icon_new_from_file(options.icon_path.c_str());
  if (!host->icon_) {
    return tl::unexpected(Error{"Failed to create GtkStatusIcon"});
  }

  if (!options.tooltip.empty()) {
    gtk_status_icon_set_tooltip_text(host->icon_, options.tooltip.c_str());
  }

  g_signal_connect(host->icon_, "activate", G_CALLBACK(on_activate),
      host.get());
  g_signal_connect(host->icon_, "popup-menu", G_CALLBACK(on_popup_menu),
      host.get());

  host->build_menu(options.menu);

  gtk_status_icon_set_visible(host->icon_, TRUE);

  return host;
}

void X11TrayHost::on_activate(GtkStatusIcon* /*icon*/, void* user_data) {
  auto* self = static_cast<X11TrayHost*>(user_data);
  if (self->on_click_) {
    self->on_click_();
  }
}

void X11TrayHost::on_popup_menu(GtkStatusIcon* icon, unsigned int button,
    unsigned int activate_time, void* user_data) {
  auto* self = static_cast<X11TrayHost*>(user_data);
  if (self->on_right_click_) {
    self->on_right_click_();
    return;
  }
  if (!self->menu_) {
    return;
  }
  gtk_menu_popup(GTK_MENU(self->menu_), nullptr, nullptr,
      gtk_status_icon_position_menu, icon, button, activate_time);
}

void X11TrayHost::on_menu_item_activate(GtkWidget* widget,
    void* user_data) {
  auto* self = static_cast<X11TrayHost*>(user_data);
  auto* id = static_cast<const char*>(
      g_object_get_data(G_OBJECT(widget), "menu-item-id"));
  if (id && self->on_menu_click_) {
    self->on_menu_click_(std::string(id));
  }
}

void X11TrayHost::build_menu(const std::vector<TrayMenuItem>& items) {
  destroy_menu();

  if (items.empty()) {
    return;
  }

  menu_items_ = items;
  menu_ = gtk_menu_new();

  for (const auto& item : menu_items_) {
    GtkWidget* menu_item;

    if (item.label.empty()) {
      menu_item = gtk_separator_menu_item_new();
    } else {
      menu_item = gtk_menu_item_new_with_label(item.label.c_str());
      gtk_widget_set_sensitive(menu_item, item.enabled ? TRUE : FALSE);

      g_object_set_data(G_OBJECT(menu_item), "menu-item-id",
          const_cast<char*>(item.id.c_str()));

      g_signal_connect(menu_item, "activate",
          G_CALLBACK(on_menu_item_activate), this);
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_), menu_item);
  }

  gtk_widget_show_all(menu_);
}

void X11TrayHost::destroy_menu() {
  if (menu_) {
    gtk_widget_destroy(menu_);
    menu_ = nullptr;
  }
  menu_items_.clear();
}

Result<void> X11TrayHost::set_icon(std::string_view icon_path) {
  if (!icon_) {
    return tl::unexpected(Error{"Tray icon not initialized"});
  }
  gtk_status_icon_set_from_file(icon_, std::string(icon_path).c_str());
  return {};
}

Result<void> X11TrayHost::set_tooltip(std::string_view tooltip) {
  if (!icon_) {
    return tl::unexpected(Error{"Tray icon not initialized"});
  }
  gtk_status_icon_set_tooltip_text(icon_, std::string(tooltip).c_str());
  return {};
}

Result<void> X11TrayHost::set_menu(std::vector<TrayMenuItem> menu) {
  if (!icon_) {
    return tl::unexpected(Error{"Tray icon not initialized"});
  }
  build_menu(menu);
  return {};
}

Result<Geometry> X11TrayHost::get_icon_rect() const {
  if (!icon_) {
    return tl::unexpected(Error{"Tray icon not initialized"});
  }
  GdkScreen* screen = nullptr;
  GdkRectangle area = {};
  if (!gtk_status_icon_get_geometry(icon_, &screen, &area, nullptr)) {
    return tl::unexpected(Error{"icon_rect_failed",
        "gtk_status_icon_get_geometry failed"});
  }
  // gtk_status_icon_get_geometry returns physical pixel coordinates,
  // but gtk_window_move uses logical coordinates. Divide by scale factor.
  int scale = 1;
  GdkDisplay* display = gdk_display_get_default();
  if (display) {
    GdkMonitor* monitor = gdk_display_get_monitor_at_point(display, area.x, area.y);
    if (monitor) {
      scale = gdk_monitor_get_scale_factor(monitor);
    }
  }
  if (scale < 1) scale = 1;
  return Geometry{area.x / scale, area.y / scale, area.width / scale, area.height / scale};
}

Result<Position> X11TrayHost::get_popup_position(int popup_width, int popup_height) const {
  if (!icon_) {
    return tl::unexpected(Error{"Tray icon not initialized"});
  }

  GdkScreen* screen = nullptr;
  GdkRectangle area = {};
  if (!gtk_status_icon_get_geometry(icon_, &screen, &area, nullptr)) {
    return tl::unexpected(Error{"icon_rect_failed",
        "gtk_status_icon_get_geometry failed"});
  }

  // Get scale factor and convert icon rect to logical coordinates
  int scale = 1;
  GdkDisplay* display = gdk_display_get_default();
  GdkMonitor* monitor = nullptr;
  if (display) {
    monitor = gdk_display_get_monitor_at_point(display, area.x, area.y);
    if (monitor) {
      scale = gdk_monitor_get_scale_factor(monitor);
    }
  }
  if (scale < 1) scale = 1;

  int icon_x = area.x / scale;
  int icon_y = area.y / scale;
  int icon_w = area.width / scale;
  int icon_h = area.height / scale;

  // Get work area in logical coordinates
  GdkRectangle workarea = {};
  if (monitor) {
    gdk_monitor_get_workarea(monitor, &workarea);
  } else {
    // Fallback: use icon position as center, no clamping possible
    return Position{icon_x + icon_w / 2 - popup_width / 2,
                    icon_y - popup_height};
  }

  int icon_cx = icon_x + icon_w / 2;
  int icon_cy = icon_y + icon_h / 2;

  int dist_top = icon_cy - workarea.y;
  int dist_bottom = (workarea.y + workarea.height) - icon_cy;
  int dist_left = icon_cx - workarea.x;
  int dist_right = (workarea.x + workarea.width) - icon_cx;

  int popup_x = 0;
  int popup_y = 0;

  int min_dist = dist_bottom;
  int edge = 0; // 0=bottom, 1=top, 2=right, 3=left

  if (dist_top < min_dist) { min_dist = dist_top; edge = 1; }
  if (dist_right < min_dist) { min_dist = dist_right; edge = 2; }
  if (dist_left < min_dist) { min_dist = dist_left; edge = 3; }

  switch (edge) {
    case 0: // taskbar at bottom
      popup_x = icon_cx - popup_width / 2;
      popup_y = icon_y - popup_height;
      break;
    case 1: // taskbar at top
      popup_x = icon_cx - popup_width / 2;
      popup_y = icon_y + icon_h;
      break;
    case 2: // taskbar at right
      popup_x = icon_x - popup_width;
      popup_y = icon_cy - popup_height / 2;
      break;
    case 3: // taskbar at left
      popup_x = icon_x + icon_w;
      popup_y = icon_cy - popup_height / 2;
      break;
  }

  // Clamp to work area
  int wa_right = workarea.x + workarea.width;
  int wa_bottom = workarea.y + workarea.height;

  if (popup_x + popup_width > wa_right)
    popup_x = wa_right - popup_width;
  if (popup_x < workarea.x)
    popup_x = workarea.x;
  if (popup_y + popup_height > wa_bottom)
    popup_y = wa_bottom - popup_height;
  if (popup_y < workarea.y)
    popup_y = workarea.y;

  return Position{popup_x, popup_y};
}

Result<void> X11TrayHost::remove() {
  if (icon_) {
    gtk_status_icon_set_visible(icon_, FALSE);
    g_object_unref(icon_);
    icon_ = nullptr;
  }
  destroy_menu();
  return {};
}

} // namespace viewshell
