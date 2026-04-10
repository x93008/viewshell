#include "test_app_harness.h"
#include <cstdlib>
#include <viewshell/application.h>
#include <viewshell/window_handle.h>

namespace viewshell {

bool HasDisplay() {
  return std::getenv("DISPLAY") != nullptr || std::getenv("WAYLAND_DISPLAY") != nullptr;
}

TestAppHarness::TestAppHarness() = default;
TestAppHarness::~TestAppHarness() = default;

bool TestAppHarness::launch_local(const std::string& entry_file) {
  AppOptions opts;
  auto app = Application::create(opts);
  if (!app) return false;

  WindowOptions win_opts;
  win_opts.asset_root = entry_file;
  auto win = app->create_window(win_opts);
  if (!win) return false;

  app_ = std::make_unique<Application>(std::move(*app));
  window_ = &*win;
  return true;
}

bool TestAppHarness::launch_remote(const std::string& url) {
  AppOptions opts;
  auto app = Application::create(opts);
  if (!app) return false;

  auto win = app->create_window({});
  if (!win) return false;

  app_ = std::make_unique<Application>(std::move(*app));
  window_ = &*win;
  return true;
}

std::string TestAppHarness::eval_js(const std::string&) { return ""; }

std::string TestAppHarness::last_page_load_stage() const {
  if (page_stages_.empty()) return "";
  return page_stages_.back();
}

std::string TestAppHarness::last_page_load_error() const { return last_error_; }

std::vector<std::string> TestAppHarness::page_load_stages() const {
  return page_stages_;
}

void TestAppHarness::set_navigation_decision(NavigationDecision decision) {
  nav_decision_ = decision;
}

Result<void> TestAppHarness::navigate(const std::string& url) {
  if (nav_decision_ == NavigationDecision::Deny) {
    return tl::unexpected(Error{"navigation_denied", "navigation was denied"});
  }
  return {};
}

void TestAppHarness::add_init_script(const std::string& script) {
  if (window_) {
    window_->add_init_script(script);
  }
}

void TestAppHarness::on_page_load(PageLoadHandler handler) {
  if (window_) {
    window_->on_page_load(std::move(handler));
  }
}

void TestAppHarness::on_navigation(NavigationHandler handler) {
  if (window_) {
    window_->set_navigation_handler(std::move(handler));
  }
}

std::vector<std::string> TestAppHarness::take_logs() {
  return std::exchange(logs_, {});
}

Result<Capabilities> TestAppHarness::host_capabilities() const {
  if (!window_) return tl::unexpected(Error{"invalid_state", "no window"});
  return window_->capabilities();
}

} // namespace viewshell
