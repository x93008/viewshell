#include <viewshell/application.h>
#include <viewshell/window_handle.h>

#include <cstdio>

#include "../common/example_asset_path.h"

int main(int argc, char* argv[]) {
  auto asset_path = viewshell::examples::resolve_example_asset_path(argv[0], "index.html");

  auto app = viewshell::Application::create({});
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s (%s)\n", app.error().code.c_str(), app.error().message.c_str());
    return 1;
  }

  viewshell::WindowOptions opts;
  opts.asset_root = asset_path;
  opts.width = 120;
  opts.height = 60;
  opts.borderless = true;
  opts.always_on_top = true;
  opts.show_in_taskbar = false;
  opts.resizable = false;
  opts.inject_window_api = true;
  opts.x = 200;
  opts.y = 200;

  auto window = app->create_window(opts);
  if (!window) {
    std::fprintf(stderr, "failed to create window: %s (%s)\n", window.error().code.c_str(), window.error().message.c_str());
    return 1;
  }
  window->set_title("Hover Test");

  auto run_result = app->run();
  if (!run_result) {
    std::fprintf(stderr, "run failed: %s (%s)\n", run_result.error().code.c_str(), run_result.error().message.c_str());
    return 1;
  }
  return *run_result;
}
