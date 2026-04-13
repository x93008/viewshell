#include <gtest/gtest.h>

#include <viewshell/application.h>

#include "../../src/viewshell/runtime_state.h"
#include "../../src/runtime/backend_runtime.h"
#include "../../src/runtime/backend_factory.h"
#include "../../src/runtime/window_host.h"

namespace viewshell {

bool HasBackendRuntimeForTest(const Application& app) {
  return static_cast<bool>(app.backend_runtime_);
}

bool HasWindowHostForTest(const Application& app) {
  return static_cast<bool>(app.window_state_) &&
      static_cast<bool>(app.window_state_->window_host);
}

bool WindowCapabilitiesAvailableForTest(const Application& app) {
  if (!app.window_state_ || !app.window_state_->window_host) {
    return false;
  }
  auto caps = app.window_state_->window_host->capabilities();
  return static_cast<bool>(caps);
}

} // namespace viewshell

TEST(ApplicationLifecycle, requires_window_before_run) {
  auto app = viewshell::Application::create({});
  ASSERT_TRUE(app);
  auto run = app->run();
  ASSERT_FALSE(run);
  EXPECT_EQ(run.error().code, "invalid_state");
}

TEST(ApplicationLifecycle, bridge_handle_exists_before_run_for_registration) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge();
  ASSERT_TRUE(bridge);
}

TEST(ApplicationLifecycle, pre_run_command_registration_succeeds) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  auto result = bridge.register_command("app.ping",
    [](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
      return viewshell::Json{{"ok", true}};
    });
  ASSERT_TRUE(result);
}

TEST(ApplicationLifecycle, application_creation_configures_backend_runtime) {
  auto app = viewshell::Application::create({});
  ASSERT_TRUE(app);
  EXPECT_TRUE(viewshell::HasBackendRuntimeForTest(*app));
}

TEST(ApplicationLifecycle, backend_factory_runtime_reports_invalid_state_before_run) {
  auto runtime = viewshell::BackendFactory::create();
  ASSERT_NE(runtime, nullptr);

  auto app_state = std::make_shared<viewshell::RuntimeAppState>();
  auto result = runtime->post(app_state, [] {});

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_state");
}

TEST(ApplicationLifecycle, duplicate_command_registration_is_rejected) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  ASSERT_TRUE(bridge.register_command("app.ping",
    [](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
      return viewshell::Json{{"ok", true}};
    }));
  auto second = bridge.register_command("app.ping",
    [](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
      return viewshell::Json{{"ok", false}};
    });
  ASSERT_FALSE(second);
  EXPECT_EQ(second.error().code, "command_already_registered");
}

TEST(ApplicationLifecycle, pre_run_window_configuration_succeeds) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  EXPECT_TRUE(window.set_size({800, 600}));
  EXPECT_TRUE(window.set_borderless(true));
}

TEST(ApplicationLifecycle, create_window_stores_window_host_not_concrete_drivers) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({});

  ASSERT_TRUE(window);
  EXPECT_TRUE(viewshell::HasWindowHostForTest(app));
  EXPECT_TRUE(viewshell::WindowCapabilitiesAvailableForTest(app));
}

TEST(ApplicationLifecycle, application_can_be_destroyed_after_window_creation) {
  auto app = viewshell::Application::create({});
  ASSERT_TRUE(app);
  ASSERT_TRUE(app->create_window({}));
}
