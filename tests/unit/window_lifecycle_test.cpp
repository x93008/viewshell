#include <gtest/gtest.h>
#include <thread>

// Test-only access for constructing handles from injected runtime state.
#define private public
#include <viewshell/application.h>
#undef private

#include "../../src/viewshell/runtime_state.h"
#include "../../src/runtime/window_host.h"
#include "runtime_test_hooks.h"

namespace {

class RecordingWindowHost final : public viewshell::WindowHost {
public:
  viewshell::Result<void> set_title(std::string_view title) override {
    last_title = std::string(title);
    return {};
  }

  viewshell::Result<void> maximize() override { return {}; }
  viewshell::Result<void> unmaximize() override { return {}; }
  viewshell::Result<void> minimize() override { return {}; }
  viewshell::Result<void> unminimize() override { return {}; }
  viewshell::Result<void> show() override { return {}; }
  viewshell::Result<void> hide() override { return {}; }
  viewshell::Result<void> focus() override { return {}; }
  viewshell::Result<void> set_size(viewshell::Size) override { return {}; }
  viewshell::Result<viewshell::Size> get_size() const override {
    return viewshell::Size{640, 480};
  }

  viewshell::Result<void> set_position(viewshell::Position) override { return {}; }
  viewshell::Result<viewshell::Position> get_position() const override {
    return viewshell::Position{10, 20};
  }

  viewshell::Result<void> set_borderless(bool) override { return {}; }
  viewshell::Result<void> set_always_on_top(bool) override { return {}; }
  viewshell::Result<void> close() override { return {}; }

  viewshell::Result<void> load_url(std::string_view url) override {
    last_url = std::string(url);
    return {};
  }

  viewshell::Result<void> load_file(std::string_view entry_file) override {
    last_file = std::string(entry_file);
    return {};
  }

  viewshell::Result<void> reload() override { return {}; }

  viewshell::Result<void> evaluate_script(std::string_view script) override {
    last_script = std::string(script);
    return {};
  }

  viewshell::Result<void> add_init_script(std::string_view script) override {
    init_scripts.push_back(std::string(script));
    return {};
  }

  viewshell::Result<void> open_devtools() override { return {}; }
  viewshell::Result<void> close_devtools() override { return {}; }

  viewshell::Result<void> on_page_load(viewshell::PageLoadHandler handler) override {
    page_load_handler = std::move(handler);
    return {};
  }

  viewshell::Result<void> set_navigation_handler(viewshell::NavigationHandler handler) override {
    navigation_handler = std::move(handler);
    return {};
  }

  viewshell::Result<viewshell::Capabilities> capabilities() const override {
    return capabilities_value;
  }

  viewshell::Result<void> register_command(std::string name, viewshell::CommandHandler handler) override {
    if (commands.count(name)) {
      return tl::unexpected(viewshell::Error{"command_already_registered", "duplicate"});
    }
    commands.emplace(std::move(name), std::move(handler));
    return {};
  }

  viewshell::Result<void> emit(std::string name, const viewshell::Json& payload) override {
    last_event_name = std::move(name);
    last_event_payload = payload;
    if (!bridge_active) {
      return tl::unexpected(viewshell::Error{"bridge_unavailable", "bridge is not active"});
    }
    return {};
  }

  std::string last_title;
  std::string last_url;
  std::string last_file;
  std::string last_script;
  std::string last_event_name;
  viewshell::Json last_event_payload;
  std::vector<std::string> init_scripts;
  viewshell::PageLoadHandler page_load_handler;
  viewshell::NavigationHandler navigation_handler;
  viewshell::Capabilities capabilities_value;
  std::unordered_map<std::string, viewshell::CommandHandler> commands;
  bool bridge_active = false;
};

} // namespace

TEST(WindowHandleLifecycle, reports_unsupported_backend_when_no_window_host_present) {
  auto state = std::make_shared<viewshell::RuntimeWindowState>();

  EXPECT_EQ(state->window_host, nullptr);

  viewshell::WindowHandle handle(state);
  auto result = handle.load_url("https://example.com");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "unsupported_by_backend");
}

TEST(WindowHandleLifecycle, delegates_window_ops_to_window_host) {
  auto state = std::make_shared<viewshell::RuntimeWindowState>();
  auto host = std::make_shared<RecordingWindowHost>();
  state->window_host = host;

  viewshell::WindowHandle handle(state);
  ASSERT_TRUE(handle.set_title("Delegated title"));

  EXPECT_EQ(host->last_title, "Delegated title");
}

TEST(WindowHandleLifecycle, delegates_webview_ops_to_window_host) {
  auto state = std::make_shared<viewshell::RuntimeWindowState>();
  auto host = std::make_shared<RecordingWindowHost>();
  state->window_host = host;

  viewshell::WindowHandle handle(state);
  ASSERT_TRUE(handle.load_url("https://example.com/runtime"));

  EXPECT_EQ(host->last_url, "https://example.com/runtime");
}

TEST(WindowHandleLifecycle, bridge_handle_delegates_registration_and_emit_to_window_host) {
  auto state = std::make_shared<viewshell::RuntimeWindowState>();
  auto host = std::make_shared<RecordingWindowHost>();
  state->window_host = host;

  viewshell::WindowHandle handle(state);
  auto bridge = handle.bridge();
  ASSERT_TRUE(bridge);

  ASSERT_TRUE(bridge->register_command("app.ping",
      [](const viewshell::Json&) -> viewshell::Result<viewshell::Json> {
        return viewshell::Json{{"ok", true}};
      }));
  EXPECT_TRUE(host->commands.count("app.ping"));

  auto emit_result = bridge->emit("native-ready", viewshell::Json{{"ok", true}});
  ASSERT_FALSE(emit_result);
  EXPECT_EQ(emit_result.error().code, "bridge_unavailable");

  host->bridge_active = true;
  ASSERT_TRUE(bridge->emit("native-ready", viewshell::Json{{"ok", true}}));
  EXPECT_EQ(host->last_event_name, "native-ready");
  EXPECT_EQ(host->last_event_payload["ok"], true);
}

TEST(WindowHandleLifecycle, reports_unsupported_backend_for_window_ops_when_no_window_host_present) {
  auto state = std::make_shared<viewshell::RuntimeWindowState>();

  viewshell::WindowHandle handle(state);
  auto result = handle.set_title("missing host");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "unsupported_by_backend");
}

TEST(WindowLifecycle, second_window_is_supported) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  auto second = app.create_window({});
  ASSERT_TRUE(second);
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

TEST(WindowLifecycle, create_window_after_run_started_is_supported) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  viewshell::MarkRunStartedForTest(app);
  auto result = app.create_window({});
  ASSERT_TRUE(result);
}

TEST(WindowLifecycle, create_window_after_close_is_supported) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  viewshell::MarkWindowClosedForTest(window);
  auto result = app.create_window({});
  ASSERT_TRUE(result);
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
