#pragma once

#include <string>
#include <vector>
#include <functional>
#include <viewshell/types.h>
#include <viewshell/options.h>
#include <viewshell/application.h>
#include <viewshell/capabilities.h>

namespace viewshell {

class TestAppHarness {
public:
  TestAppHarness();
  ~TestAppHarness();

  bool launch_local(const std::string& entry_file);
  bool launch_remote(const std::string& url);
  std::string eval_js(const std::string& expression);
  std::string last_page_load_stage() const;
  std::string last_page_load_error() const;
  std::vector<std::string> page_load_stages() const;
  void set_navigation_decision(NavigationDecision decision);
  Result<void> navigate(const std::string& url);
  void add_init_script(const std::string& script);
  void on_page_load(PageLoadHandler handler);
  void on_navigation(NavigationHandler handler);
  std::vector<std::string> take_logs();
  Result<Capabilities> host_capabilities() const;
  WindowHandle& host_window() { return *window_; }

private:
  std::unique_ptr<Application> app_;
  WindowHandle* window_ = nullptr;
  std::vector<std::string> page_stages_;
  std::string last_error_;
  NavigationDecision nav_decision_ = NavigationDecision::Allow;
  std::vector<std::string> logs_;
};

bool HasDisplay();

} // namespace viewshell
