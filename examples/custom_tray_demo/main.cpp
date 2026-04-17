#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>
#include <viewshell/tray_handle.h>

#include <cstdio>
#include <memory>

#include "../common/example_asset_path.h"

struct AppState {
  viewshell::WindowHandle main_window;
  viewshell::WindowHandle menu_window;
  std::shared_ptr<viewshell::TrayHandle> tray;
  bool menu_visible = false;
};

int main(int argc, char* argv[]) {
  auto asset_path = viewshell::examples::resolve_example_asset_path(argv[0], "index.html");
  auto menu_path = viewshell::examples::resolve_example_asset_path(argv[0], "menu.html");
  auto icon_path = viewshell::examples::resolve_example_asset_path(argv[0], "tray_icon.png");

  auto app = viewshell::Application::create({});
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s (%s)\n", app.error().code.c_str(), app.error().message.c_str());
    return 1;
  }

  // Main window
  viewshell::WindowOptions main_opts;
  main_opts.asset_root = asset_path;
  main_opts.width = 400;
  main_opts.height = 240;
  main_opts.x = 200;
  main_opts.y = 200;

  auto main_window = app->create_window(main_opts);
  if (!main_window) {
    std::fprintf(stderr, "failed to create main window: %s (%s)\n", main_window.error().code.c_str(), main_window.error().message.c_str());
    return 1;
  }
  main_window->set_title("Custom Tray Menu Demo");

  // Popup menu window (hidden initially)
  viewshell::WindowOptions menu_opts;
  menu_opts.asset_root = menu_path;
  menu_opts.width = 200;
  menu_opts.height = 170;
  menu_opts.borderless = true;
  menu_opts.always_on_top = true;
  menu_opts.show_in_taskbar = false;
  menu_opts.resizable = false;

  auto menu_window = app->create_window(menu_opts);
  if (!menu_window) {
    std::fprintf(stderr, "failed to create menu window: %s (%s)\n", menu_window.error().code.c_str(), menu_window.error().message.c_str());
    return 1;
  }
  menu_window->hide();

  auto shared = std::make_shared<AppState>(AppState{*main_window, *menu_window, nullptr, false});

  // Register menu click handler on the popup window
  auto menu_bridge = menu_window->bridge();
  if (menu_bridge) {
    menu_bridge->register_command("menu.click",
      [shared](const viewshell::Json& args) -> viewshell::Result<viewshell::Json> {
        auto id = args.value("id", std::string(""));
        shared->menu_window.hide();
        shared->menu_visible = false;

        if (id == "show") {
          shared->main_window.show();
          shared->main_window.focus();
        } else if (id == "about") {
          shared->main_window.set_title("Custom Tray Demo — viewshell custom tray menu example");
          shared->main_window.show();
          shared->main_window.focus();
        } else if (id == "toggle") {
          shared->main_window.set_title("Custom Tray Demo — toggled!");
        } else if (id == "quit") {
          shared->menu_window.close();
          shared->main_window.close();
        }
        return viewshell::Json{{"ok", true}};
      });
  }

  // Create tray — use shared state indirection so callbacks can access tray handle
  viewshell::TrayOptions tray_opts;
  tray_opts.icon_path = icon_path;
  tray_opts.tooltip = "Custom Tray Menu";
  tray_opts.on_click = [shared]() {
    if (shared->menu_visible) {
      shared->menu_window.hide();
      shared->menu_visible = false;
    } else {
      shared->main_window.show();
      shared->main_window.focus();
    }
  };
  tray_opts.on_right_click = [shared]() {
    if (!shared->tray) return;
    auto rect = shared->tray->get_icon_rect();
    int popup_w = 200;
    int popup_h = 170;
    int x = 0, y = 0;
    std::string debug = "get_icon_rect: ";
    if (rect) {
      debug += "x=" + std::to_string(rect->x) + " y=" + std::to_string(rect->y)
        + " w=" + std::to_string(rect->width) + " h=" + std::to_string(rect->height)
        + " (scaled)";
      x = rect->x + (rect->width - popup_w) / 2;
      y = rect->y - popup_h;
      if (y < 0) y = rect->y + rect->height;
    } else {
      debug += "FAILED: " + rect.error().message;
    }
    debug += " -> pos=(" + std::to_string(x) + "," + std::to_string(y) + ")";
    shared->main_window.set_title(debug);
    shared->menu_window.show();
    shared->menu_window.set_geometry({x, y, popup_w, popup_h});
    shared->menu_window.focus();
    shared->menu_visible = true;
  };

  auto tray = app->create_tray(tray_opts);
  if (!tray) {
    std::fprintf(stderr, "failed to create tray: %s (%s)\n", tray.error().code.c_str(), tray.error().message.c_str());
    return 1;
  }
  shared->tray = std::make_shared<viewshell::TrayHandle>(*tray);

  auto run_result = app->run();
  if (!run_result) {
    std::fprintf(stderr, "run failed: %s (%s)\n", run_result.error().code.c_str(), run_result.error().message.c_str());
    return 1;
  }
  return *run_result;
}
