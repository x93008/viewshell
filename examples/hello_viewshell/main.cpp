#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>
#include <cstdio>

#include "../common/example_asset_path.h"

int main(int argc, char* argv[]) {
  auto asset_path = viewshell::examples::resolve_example_asset_path(argv[0], "index.html");

  viewshell::AppOptions app_opts;

  auto app = viewshell::Application::create(app_opts);
  if (!app) {
    std::fprintf(stderr, "failed to create application: %s (%s)\n",
        app.error().code.c_str(), app.error().message.c_str());
    return 1;
  }

  viewshell::WindowOptions win_opts;
  win_opts.asset_root = asset_path;
  win_opts.width = 640;
  win_opts.height = 480;

  auto win = app->create_window(win_opts);
  if (!win) {
    std::fprintf(stderr, "failed to create window: %s (%s)\n",
        win.error().code.c_str(), win.error().message.c_str());
    return 1;
  }

  win->set_title("Hello Viewshell");
  win->add_init_script("window.__viewshellInitScriptRan = 'yes';");
  win->on_page_load([](const viewshell::PageLoadEvent& event) {
    std::fprintf(stderr, "page-load: url=%s stage=%s error=%s\n",
        event.url.c_str(),
        event.stage.c_str(),
        event.error_code ? event.error_code->c_str() : "none");
  });

  auto bridge = win->bridge();
  win->set_navigation_handler([bridge](const viewshell::NavigationRequest& request) mutable {
    bool allow =
        request.url.rfind("file://", 0) == 0 ||
        request.url.rfind("viewshell://app/", 0) == 0 ||
        request.url.rfind("https://github.com/x93008/viewshell", 0) == 0;
    if (bridge) {
      (void)bridge->emit("navigation-decision", viewshell::Json{{"url", request.url}, {"decision", allow ? "allow" : "deny"}});
    }
    return allow ? viewshell::NavigationDecision::Allow : viewshell::NavigationDecision::Deny;
  });

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
    std::fprintf(stderr, "run failed: %s (%s)\n",
        run_result.error().code.c_str(), run_result.error().message.c_str());
    return 1;
  }

  return *run_result;
}
