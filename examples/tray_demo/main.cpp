#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <viewshell/tray_handle.h>

#include <cstdio>

#include "../common/example_asset_path.h"

int main(int argc, char* argv[]) {
  auto asset_path = viewshell::examples::resolve_example_asset_path(argv[0], "index.html");
  auto icon_path = viewshell::examples::resolve_example_asset_path(argv[0], "tray_icon.png");

  auto app = viewshell::Application::create({});
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s (%s)\n", app.error().code.c_str(), app.error().message.c_str());
    return 1;
  }

  viewshell::WindowOptions win_opts;
  win_opts.asset_root = asset_path;
  win_opts.width = 400;
  win_opts.height = 240;
  win_opts.x = 200;
  win_opts.y = 200;

  auto window = app->create_window(win_opts);
  if (!window) {
    std::fprintf(stderr, "failed to create window: %s (%s)\n", window.error().code.c_str(), window.error().message.c_str());
    return 1;
  }
  window->set_title("Tray Demo");

  auto win = *window;

  viewshell::TrayOptions tray_opts;
  tray_opts.icon_path = icon_path;
  tray_opts.tooltip = "Tray Demo";
  tray_opts.menu = {
    {"about", "About"},
    {"", ""},
    {"quit", "Quit"},
  };
  tray_opts.on_click = [win]() mutable {
    win.show();
    win.focus();
  };
  tray_opts.on_menu_click = [win](const std::string& id) mutable {
    if (id == "about") {
      win.set_title("Tray Demo — About: viewshell tray example");
      win.show();
      win.focus();
    } else if (id == "quit") {
      win.close();
    }
  };

  auto tray = app->create_tray(tray_opts);
  if (!tray) {
    std::fprintf(stderr, "failed to create tray: %s (%s)\n", tray.error().code.c_str(), tray.error().message.c_str());
    return 1;
  }

  auto run_result = app->run();
  if (!run_result) {
    std::fprintf(stderr, "run failed: %s (%s)\n", run_result.error().code.c_str(), run_result.error().message.c_str());
    return 1;
  }
  return *run_result;
}
