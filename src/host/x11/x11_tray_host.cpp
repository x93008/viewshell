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
  GdkRectangle area = {};
  if (!gtk_status_icon_get_geometry(icon_, nullptr, &area, nullptr)) {
    return tl::unexpected(Error{"icon_rect_failed",
        "gtk_status_icon_get_geometry failed"});
  }
  return Geometry{area.x, area.y, area.width, area.height};
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
