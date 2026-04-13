#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>
#include <cstdio>
#include <filesystem>

int main(int argc, char* argv[]) {
  std::string exe_arg(argv[0]);
  auto exe_dir = std::filesystem::canonical(std::filesystem::path(exe_arg)).parent_path();
  auto asset_path = exe_dir / "app" / "index.html";

  viewshell::AppOptions app_opts;

  auto app = viewshell::Application::create(app_opts);
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s\n", app.error().code.c_str());
    return 1;
  }

  viewshell::WindowOptions win_opts;
  win_opts.asset_root = asset_path.string();
  win_opts.width = 640;
  win_opts.height = 480;

  auto win = app->create_window(win_opts);
  if (!win) {
    std::fprintf(stderr, "failed to create window: %s\n", win.error().code.c_str());
    return 1;
  }

  win->set_title("Hello Viewshell");

  auto bridge = win->bridge();
  if (bridge) {
    bridge->register_command("app.ping",
      [](const viewshell::Json& args) -> viewshell::Result<viewshell::Json> {
        return viewshell::Json{{"pong", args.value("value", 0)}};
      });
    bridge->register_command("app.fail",
      [](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
        return tl::unexpected(viewshell::Error{"demo_failure", "demo reject path"});
      });
  }

  auto run_result = app->run();
  if (!run_result) {
    std::fprintf(stderr, "run failed: %s\n", run_result.error().code.c_str());
    return 1;
  }

  return *run_result;
}
