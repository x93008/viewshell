#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <viewshell/tray_options.h>
#include <viewshell/types.h>

#include "runtime/tray_host.h"

typedef struct _GtkStatusIcon GtkStatusIcon;
typedef struct _GtkWidget GtkWidget;

namespace viewshell {

class X11TrayHost final : public TrayHost {
public:
  static Result<std::shared_ptr<X11TrayHost>> create(const TrayOptions& options);

  ~X11TrayHost() override;

  Result<void> set_icon(std::string_view icon_path) override;
  Result<void> set_tooltip(std::string_view tooltip) override;
  Result<void> set_menu(std::vector<TrayMenuItem> menu) override;
  Result<Geometry> get_icon_rect() const override;
  Result<void> remove() override;

private:
  X11TrayHost();

  void build_menu(const std::vector<TrayMenuItem>& items);
  void destroy_menu();

  static void on_activate(GtkStatusIcon* icon, void* user_data);
  static void on_popup_menu(GtkStatusIcon* icon, unsigned int button,
      unsigned int activate_time, void* user_data);
  static void on_menu_item_activate(GtkWidget* widget, void* user_data);

  GtkStatusIcon* icon_ = nullptr;
  GtkWidget* menu_ = nullptr;
  std::vector<TrayMenuItem> menu_items_;
  std::function<void()> on_click_;
  std::function<void()> on_right_click_;
  std::function<void(const std::string& id)> on_menu_click_;
};

} // namespace viewshell
