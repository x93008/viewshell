#include <gtest/gtest.h>
#include <viewshell/application.h>

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
