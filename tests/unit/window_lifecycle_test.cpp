#include <gtest/gtest.h>
#include <thread>
#include <viewshell/application.h>
#include "runtime_test_hooks.h"

TEST(WindowLifecycle, second_window_is_rejected) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  auto second = app.create_window({});
  ASSERT_FALSE(second);
  EXPECT_EQ(second.error().code, "multiple_windows_unsupported");
}

TEST(WindowLifecycle, closed_handle_reports_window_closed) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  viewshell::MarkWindowClosedForTest(window);
  auto result = window.set_borderless(true);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "window_closed");
}

TEST(WindowLifecycle, closed_bridge_handle_reports_window_closed) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  viewshell::MarkWindowClosedForTest(window);
  auto result = bridge.emit("native-ready", nlohmann::json::object());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "window_closed");
}

TEST(WindowLifecycle, closed_bridge_registration_reports_window_closed) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  viewshell::MarkWindowClosedForTest(window);
  auto result = bridge.register_command("app.ping",
    [](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
      return viewshell::Json{{"ok", true}};
    });
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "window_closed");
}

TEST(WindowLifecycle, post_before_run_is_invalid_state) {
  auto app = viewshell::Application::create({}).value();
  auto result = app.post([] {});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_state");
}

TEST(WindowLifecycle, post_after_run_started_executes_task) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkRunStartedForTest(app);
  bool called = false;
  ASSERT_TRUE(app.post([&] { called = true; }));
  viewshell::PumpPostedTasksForTest(app);
  EXPECT_TRUE(called);
}

TEST(WindowLifecycle, post_from_worker_thread_after_run_started_succeeds) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkRunStartedForTest(app);
  bool called = false;

  std::thread worker([&] {
    ASSERT_TRUE(app.post([&] { called = true; }));
  });
  worker.join();
  viewshell::PumpPostedTasksForTest(app);
  EXPECT_TRUE(called);
}

TEST(WindowLifecycle, post_after_shutdown_is_invalid_state) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkShutdownStartedForTest(app);
  auto result = app.post([] {});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_state");
}

TEST(WindowLifecycle, posted_exceptions_are_caught) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkRunStartedForTest(app);
  bool still_running = false;
  ASSERT_TRUE(app.post([] { throw std::runtime_error("boom"); }));
  ASSERT_TRUE(app.post([&] { still_running = true; }));
  viewshell::PumpPostedTasksForTest(app);
  EXPECT_FALSE(viewshell::TakeRuntimeLogsForTest(app).empty());
  EXPECT_TRUE(still_running);
}

TEST(WindowLifecycle, create_window_after_run_started_is_rejected_as_multiple_windows_unsupported) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkRunStartedForTest(app);
  auto result = app.create_window({});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "multiple_windows_unsupported");
}

TEST(WindowLifecycle, create_window_after_close_is_rejected_as_multiple_windows_unsupported) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  viewshell::MarkWindowClosedForTest(window);
  auto result = app.create_window({});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "multiple_windows_unsupported");
}

TEST(WindowLifecycle, close_starts_shutdown_and_run_exits) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  viewshell::EnterRunLoopForTest(app);
  viewshell::ArmCloseAcknowledgementForTest(window);
  ASSERT_TRUE(window.close());
  auto result = viewshell::FinishRunForTest(app);
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), 0);
}

TEST(WindowLifecycle, run_returns_exit_code_once_shutdown_started) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkShutdownStartedForTest(app);
  auto result = app.run();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), 0);
}

TEST(WindowLifecycle, emit_before_bridge_activation_is_unavailable) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  auto result = bridge.emit("native-ready", nlohmann::json::object());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "bridge_unavailable");
}
