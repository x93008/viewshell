#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <cstdio>
#include <filesystem>

int main(int argc, char* argv[]) {
  std::string exe_arg(argv[0]);
  auto exe_dir = std::filesystem::canonical(std::filesystem::path(exe_arg)).parent_path();
  auto asset_path = exe_dir / "app" / "index.html";
  if (!std::filesystem::exists(asset_path)) {
    asset_path = exe_dir.parent_path() / "app" / "index.html";
  }

  auto app = viewshell::Application::create(viewshell::AppOptions{});
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s\n", app.error().code.c_str());
    return 1;
  }

  viewshell::WindowOptions win_opts;
  win_opts.asset_root = asset_path.string();
  win_opts.width = 400;
  win_opts.height = 400;
  win_opts.borderless = true;

  auto win = app->create_window(win_opts);
  if (!win) {
    std::fprintf(stderr, "failed to create window: %s\n", win.error().code.c_str());
    return 1;
  }

  win->set_title("Circle Window");

  auto run_result = app->run();
  if (!run_result) {
    std::fprintf(stderr, "run failed: %s\n", run_result.error().code.c_str());
    return 1;
  }

  return *run_result;
}
