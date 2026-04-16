#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>

#include <cstdio>
#include <memory>

#include "../common/example_asset_path.h"

struct DemoState {
  viewshell::WindowHandle floating_window;
  bool floating_visible = false;
};

int main(int argc, char* argv[]) {
  auto asset_path = viewshell::examples::resolve_example_asset_path(argv[0], "index.html");

  auto app = viewshell::Application::create({});
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s (%s)\n", app.error().code.c_str(), app.error().message.c_str());
    return 1;
  }

  viewshell::WindowOptions floating_opts;
  floating_opts.asset_root = asset_path;
  floating_opts.width = 67;
  floating_opts.height = 16;
  floating_opts.borderless = true;
  floating_opts.always_on_top = true;
  floating_opts.show_in_taskbar = false;
  floating_opts.resizable = false;
  floating_opts.x = 180;
  floating_opts.y = 180;

  auto floating = app->create_window(floating_opts);
  if (!floating) {
    std::fprintf(stderr, "failed to create floating window: %s (%s)\n", floating.error().code.c_str(), floating.error().message.c_str());
    return 1;
  }
  floating->set_title("Floating Panel");
  floating->add_init_script("window.__demoRole='floating';");
  floating->hide();

  viewshell::WindowOptions main_opts;
  main_opts.asset_root = asset_path;
  main_opts.width = 420;
  main_opts.height = 180;
  main_opts.x = 120;
  main_opts.y = 120;

  auto main_window = app->create_window(main_opts);
  if (!main_window) {
    std::fprintf(stderr, "failed to create main window: %s (%s)\n", main_window.error().code.c_str(), main_window.error().message.c_str());
    return 1;
  }
  main_window->set_title("Floating Demo Control");
  main_window->add_init_script("window.__demoRole='main';");

  auto shared = std::make_shared<DemoState>(DemoState{*floating, false});

  auto main_bridge = main_window->bridge();
  if (main_bridge) {
    main_bridge->register_command("demo.toggleFloating",
      [shared](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
        shared->floating_visible = !shared->floating_visible;
        if (shared->floating_visible) {
          auto show_result = shared->floating_window.show();
          if (!show_result) return tl::unexpected(show_result.error());
          auto focus_result = shared->floating_window.focus();
          if (!focus_result) return tl::unexpected(focus_result.error());
        } else {
          auto hide_result = shared->floating_window.hide();
          if (!hide_result) return tl::unexpected(hide_result.error());
        }
        return viewshell::Json{{"visible", shared->floating_visible}};
      });
  }

  auto floating_bridge = floating->bridge();
  if (floating_bridge) {
    floating_bridge->register_command("demo.floatingTransition",
      [shared](const viewshell::Json& args) -> viewshell::Result<viewshell::Json> {
        auto state = args.value("state", std::string("compact"));
        viewshell::Size size{67, 16};
        if (state == "hover") size = {81, 24};
        if (state == "expanded") size = {424, 70};
        auto resize = shared->floating_window.set_size(size);
        if (!resize) return tl::unexpected(resize.error());
        return viewshell::Json{{"state", state}, {"width", size.width}, {"height", size.height}};
      });
  }

  auto run_result = app->run();
  if (!run_result) {
    std::fprintf(stderr, "run failed: %s (%s)\n", run_result.error().code.c_str(), run_result.error().message.c_str());
    return 1;
  }
  return *run_result;
}
