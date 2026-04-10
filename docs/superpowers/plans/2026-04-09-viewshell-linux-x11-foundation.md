# Viewshell Linux X11 Foundation Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first working Linux X11 single-window Viewshell runtime that can load local frontend assets under `viewshell://app/`, open untrusted remote URLs without bridge injection, expose built-in window APIs to the frontend, and support custom native `invoke` plus native-to-web events.

**Architecture:** This slice implements the public handle types and the Linux X11 backend together, using WebKitGTK as the only accepted engine family in v1. The bridge is split into a built-in fast path for library-owned window/WebView operations and a generic invoke/event path for app-defined commands and native events, with trust gating applied before dispatch.

**Tech Stack:** C++20, CMake, GTK3/X11, WebKitGTK, CTest, pkg-config

---

This plan is intentionally limited to the first executable sub-project from the broader spec in `docs/superpowers/specs/2026-04-09-viewshell-design.md`.
Follow-up plans should cover Windows/macOS backends and post-MVP hardening separately.

## File Structure

### Public API

- Create: `include/viewshell/application.h`
- Create: `include/viewshell/window_handle.h`
- Create: `include/viewshell/bridge_handle.h`
- Create: `include/viewshell/options.h`
- Create: `include/viewshell/capabilities.h`
- Create: `include/viewshell/types.h`

Responsibilities:

- `application.h`: `Application`, `create_window`, `post`, `run`
- `window_handle.h`: stable window handle API, lifecycle-safe operations
- `bridge_handle.h`: command registration and native event emission
- `options.h`: `AppOptions`, `WindowOptions`, normalized config ownership
- `capabilities.h`: normative host/frontend capability shape
- `types.h`: `Result`, `Size`, `Position`, `PageLoadEvent`, `NavigationRequest`, `NavigationDecision`

### Core Runtime

- Create: `src/viewshell/application.cpp`
- Create: `src/viewshell/window_handle.cpp`
- Create: `src/viewshell/bridge_handle.cpp`
- Create: `src/viewshell/runtime_state.h`
- Create: `src/viewshell/public_types.cpp`

Responsibilities:

- runtime-owned handle storage
- shared runtime state for `Application`, `WindowHandle`, and `BridgeHandle`
- single-window lifecycle enforcement
- `Application::post(...)` queueing and main-loop handoff
- stable handle behavior after close

### Linux X11 Backend

- Create: `src/platform/linux_x11/window_driver.h`
- Create: `src/platform/linux_x11/window_driver.cpp`
- Create: `src/platform/linux_x11/native_window_handle.h`
- Create: `src/platform/linux_x11/webview_driver.h`
- Create: `src/platform/linux_x11/webview_driver.cpp`
- Create: `src/platform/linux_x11/kernel_resolver.h`
- Create: `src/platform/linux_x11/kernel_resolver.cpp`
- Create: `src/platform/linux_x11/bridge_driver.h`
- Create: `src/platform/linux_x11/bridge_driver.cpp`
- Create: `src/platform/linux_x11/request_tracker.h`
- Create: `src/platform/linux_x11/request_tracker.cpp`
- Create: `src/platform/linux_x11/builtin_dispatcher.h`
- Create: `src/platform/linux_x11/builtin_dispatcher.cpp`
- Create: `src/platform/linux_x11/invoke_bus.h`
- Create: `src/platform/linux_x11/invoke_bus.cpp`
- Create: `src/platform/linux_x11/trust_gate.h`
- Create: `src/platform/linux_x11/trust_gate.cpp`
- Create: `src/platform/linux_x11/resource_protocol.h`
- Create: `src/platform/linux_x11/resource_protocol.cpp`
- Create: `src/platform/linux_x11/drag_context.h`

Responsibilities:

- `window_driver.*`: GTK/X11 native window creation and window operations
- `native_window_handle.h`: backend-shared attachment target type owned by `WindowDriver` and borrowed by `WebviewDriver`
- `webview_driver.*`: WebKitGTK attach, navigation, init scripts, script eval, page callbacks
- `kernel_resolver.*`: deterministic `engine_path` / `webkitgtk` candidate probing
- `bridge_driver.*`: script-message transport attachment and page-generation resets
- `request_tracker.*`: request ids, deadlines, bridge generation invalidation
- `builtin_dispatcher.*`: `Viewshell.window.*` and `Viewshell.webview.*` operation dispatch
- `invoke_bus.*`: custom `invoke(...)` and event delivery path
- `trust_gate.*`: local full bridge vs trusted remote reduced bridge vs untrusted no-bridge
- `resource_protocol.*`: `viewshell://app/` serving under `asset_root`
- `drag_context.h`: native drag metadata shared between bridge and window driver

### Frontend Bridge Assets

- Create: `src/runtime/js/viewshell_init.js`
- Create: `src/runtime/js/viewshell_window.js`
- Create: `src/runtime/js/viewshell_webview.js`
- Create: `src/runtime/js/viewshell_invoke.js`
- Create: `src/runtime/js/viewshell_events.js`

Responsibilities:

- bootstrap `window.Viewshell`
- expose built-in Promise APIs
- expose reduced trusted-remote surface
- decode native event payloads

### Build and Tests

- Create: `CMakeLists.txt`
- Create: `cmake/ViewshellDeps.cmake`
- Create: `third_party/tl_expected/include/tl/expected.hpp`
- Create: `tests/CMakeLists.txt`
- Create: `tests/unit/application_create_test.cpp`
- Create: `tests/unit/application_lifecycle_test.cpp`
- Create: `tests/unit/window_lifecycle_test.cpp`
- Create: `tests/unit/runtime_test_hooks.h`
- Create: `tests/unit/runtime_test_hooks.cpp`
- Create: `tests/unit/public_types_test_api.h`
- Create: `tests/unit/kernel_resolver_test_support.h`
- Create: `tests/integration/window_driver_smoke_test.cpp`
- Create: `tests/integration/window_driver_test_hooks.h`
- Create: `tests/unit/request_tracker_test.cpp`
- Create: `tests/unit/trust_gate_test.cpp`
- Create: `tests/unit/kernel_resolver_test.cpp`
- Create: `tests/unit/resource_protocol_test.cpp`
- Create: `tests/unit/webview_driver_test.cpp`
- Create: `tests/unit/builtin_dispatcher_test.cpp`
- Create: `tests/integration/local_app_smoke_test.cpp`
- Create: `tests/integration/remote_untrusted_test.cpp`
- Create: `tests/integration/test_app_harness.h`
- Create: `tests/integration/test_app_harness.cpp`
- Create: `tests/fixtures/local_app/index.html`
- Create: `tests/fixtures/local_app/main.js`
- Create: `tests/fixtures/local_app/blob.unknownext`
- Create: `tests/fixtures/alt_app/index.html`
- Create: `examples/linux_x11_smoke/main.cpp`
- Create: `examples/linux_x11_smoke/app/index.html`
- Create: `examples/linux_x11_smoke/app/main.js`

Responsibilities:

- native unit tests for deterministic subsystems
- test-only runtime lifecycle and main-loop control hooks
- integration tests for local trusted app and remote untrusted page behavior
- harness helpers for integration navigation and script evaluation checks
- smoke app for manual verification and size checks

## Chunk 1: Runtime Skeleton And Lifecycle

### Task 1: Scaffold CMake And Test Harness

**Files:**
- Create: `CMakeLists.txt`
- Create: `cmake/ViewshellDeps.cmake`
- Create: `third_party/tl_expected/include/tl/expected.hpp`
- Create: `tests/CMakeLists.txt`
- Create: `include/viewshell/application.h`
- Create: `src/viewshell/application.cpp`
- Create: `tests/unit/application_create_test.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST(ApplicationCreate, rejects_invalid_config_before_runtime_boot) {
  viewshell::AppOptions options;
  options.bridge_timeout_ms = 0;
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake -S . -B build && cmake --build build --target application_create_test && ctest --test-dir build -R ApplicationCreate --output-on-failure`
Expected: FAIL because the project, test target, and public headers do not exist yet.

- [ ] **Step 3: Write minimal build and test wiring**

```cmake
add_library(viewshell STATIC)
include(cmake/ViewshellDeps.cmake)
target_compile_features(viewshell PUBLIC cxx_std_20)
target_sources(viewshell PRIVATE src/viewshell/application.cpp)
target_include_directories(viewshell PUBLIC include)
target_link_libraries(viewshell PUBLIC nlohmann_json::nlohmann_json)
enable_testing()
add_subdirectory(tests)
```

```cmake
# in cmake/ViewshellDeps.cmake
find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
target_include_directories(viewshell PUBLIC third_party/tl_expected/include)

# vendor tl::expected v1.1.0 single-header release from
# https://github.com/TartanLlama/expected/releases/tag/v1.1.0
# into third_party/tl_expected/include/tl/expected.hpp
# by copying upstream `include/tl/expected.hpp` from that exact tag.

# in tests/CMakeLists.txt
include(GoogleTest)
add_executable(application_create_test unit/application_create_test.cpp)
target_link_libraries(application_create_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(application_create_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

```cpp
struct Error { std::string code; std::string message; nlohmann::json details; };
template <typename T> using Result = tl::expected<T, Error>;
struct AppOptions { int bridge_timeout_ms = 5000; };

class Application {
public:
  static Result<Application> create(const AppOptions&);
};
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake -S . -B build && cmake --build build --target application_create_test && ctest --test-dir build -R ApplicationCreate --output-on-failure`
Expected: PASS once the minimal app factory stub is in place.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt cmake/ViewshellDeps.cmake third_party/tl_expected/include/tl/expected.hpp tests/CMakeLists.txt include/viewshell/application.h src/viewshell/application.cpp tests/unit/application_create_test.cpp
git commit -m "build: add cmake and test harness skeleton"
```

### Task 2: Freeze Public Types And Config Contracts

**Files:**
- Create: `include/viewshell/types.h`
- Create: `include/viewshell/options.h`
- Create: `include/viewshell/capabilities.h`
- Create: `src/viewshell/public_types.cpp`
- Create: `tests/unit/public_types_test_api.h`
- Modify: `include/viewshell/application.h`
- Modify: `src/viewshell/application.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/unit/application_create_test.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST(ApplicationCreate, rejects_invalid_trusted_origin_strings) {
  viewshell::AppOptions options;
  options.trusted_origins = {"not-an-origin"};
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}
```

```cpp
TEST(ApplicationCreate, normalizes_default_ports_and_deduplicates_origins) {
  viewshell::AppOptions options;
  options.trusted_origins = {
    "https://example.com:443",
    "https://example.com"
  };
  auto normalized = viewshell::detail::normalize_app_options_for_test(options);
  ASSERT_TRUE(normalized);
  EXPECT_EQ(normalized->trusted_origins.size(), 1);
  EXPECT_EQ(normalized->trusted_origins.front(), "https://example.com");
}
```

```cpp
TEST(ApplicationCreate, rejects_negative_bridge_timeout) {
  viewshell::AppOptions options;
  options.bridge_timeout_ms = -1;
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target application_create_test && ctest --test-dir build -R ApplicationCreate --output-on-failure`
Expected: FAIL because `AppOptions`, `Result`, and validation types are incomplete.

- [ ] **Step 3: Define shared value and result types**

```cpp
using Json = nlohmann::json;

struct Error {
  std::string code;
  std::string message;
  Json details;
};

template <typename T>
using Result = tl::expected<T, Error>;

struct Size { int width; int height; };
struct Position { int x; int y; };
enum class NavigationDecision { Allow, Deny };
```

- [ ] **Step 4: Define options, capabilities, and normalization seam**

```cpp
struct AppOptions {
  int bridge_timeout_ms = 5000;
  std::vector<std::string> trusted_origins;
  std::optional<std::string> require_engine;
  std::optional<std::string> engine_path;
};

struct WindowOptions {
  std::optional<std::string> asset_root;
  int width = 800;
  int height = 600;
  std::optional<int> x;
  std::optional<int> y;
  bool borderless = false;
  bool always_on_top = false;
};

struct NormalizedAppOptions {
  int bridge_timeout_ms;
  std::vector<std::string> trusted_origins;
  std::optional<std::string> require_engine;
  std::optional<std::string> engine_path;
};

struct PageLoadEvent {
  std::string url;
  std::string stage;
  std::optional<std::string> error_code;
};

struct NavigationRequest {
  std::string url;
};

using PageLoadHandler = std::function<void(const PageLoadEvent&)>;
using NavigationHandler = std::function<NavigationDecision(const NavigationRequest&)>;
using CommandHandler = std::function<Result<Json>(const Json&)>;

struct Capabilities {
  struct Window { bool borderless; bool transparent; bool always_on_top; bool native_drag; } window;
  struct Webview { bool devtools; bool resource_protocol; bool script_eval; } webview;
  struct Bridge { bool invoke; bool native_events; } bridge;
};
```

```cpp
// Application::create validates:
// - bridge_timeout_ms > 0
// - trusted_origins are normalized exact origins
// - duplicate trusted origins are deduplicated after normalization
// - `tests/unit/public_types_test_api.h` exposes
//   `viewshell::detail::normalize_app_options_for_test(...)` for unit tests only
// - `src/viewshell/public_types.cpp` owns that helper and keeps it out of the installed public headers
// - production code stores `NormalizedAppOptions` privately inside the application runtime state

namespace viewshell::detail {
  Result<NormalizedAppOptions> normalize_app_options_for_test(const AppOptions&);
}
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/viewshell/public_types.cpp)
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cmake --build build --target application_create_test && ctest --test-dir build -R ApplicationCreate --output-on-failure`
Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt include/viewshell/types.h include/viewshell/options.h include/viewshell/capabilities.h include/viewshell/application.h src/viewshell/application.cpp src/viewshell/public_types.cpp tests/unit/public_types_test_api.h tests/unit/application_create_test.cpp
git commit -m "feat: define public types options and capabilities"
```

### Task 3: Add Application, WindowHandle, And BridgeHandle Skeleton

**Files:**
- Modify: `include/viewshell/application.h`
- Create: `include/viewshell/window_handle.h`
- Create: `include/viewshell/bridge_handle.h`
- Modify: `src/viewshell/application.cpp`
- Create: `src/viewshell/window_handle.cpp`
- Create: `src/viewshell/bridge_handle.cpp`
- Modify: `tests/CMakeLists.txt`
- Modify: `CMakeLists.txt`
- Create: `tests/unit/application_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(ApplicationLifecycle, requires_window_before_run) {
  auto app = viewshell::Application::create({});
  ASSERT_TRUE(app);
  auto run = app->run();
  ASSERT_FALSE(run);
  EXPECT_EQ(run.error().code, "invalid_state");
}
```

```cpp
TEST(ApplicationLifecycle, bridge_handle_exists_before_run_for_registration) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge();
  ASSERT_TRUE(bridge);
}
```

```cpp
TEST(ApplicationLifecycle, pre_run_command_registration_succeeds) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  auto result = bridge.register_command("app.ping", [](const Json&) -> Result<Json> { return Json{{"ok", true}}; });
  ASSERT_TRUE(result);
}
```

```cpp
TEST(ApplicationLifecycle, duplicate_command_registration_is_rejected) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  ASSERT_TRUE(bridge.register_command("app.ping", [](const Json&) -> Result<Json> { return Json{{"ok", true}}; }));
  auto second = bridge.register_command("app.ping", [](const Json&) -> Result<Json> { return Json{{"ok", false}}; });
  ASSERT_FALSE(second);
  EXPECT_EQ(second.error().code, "command_already_registered");
}
```

```cpp
TEST(ApplicationLifecycle, pre_run_window_configuration_succeeds) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  EXPECT_TRUE(window.set_size({800, 600}));
  EXPECT_TRUE(window.set_borderless(true));
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build --target application_lifecycle_test && ctest --test-dir build -R ApplicationLifecycle --output-on-failure`
Expected: FAIL because public handle APIs and lifecycle rules are undefined.

- [ ] **Step 3: Write minimal implementation**

```cpp
class Application {
public:
  static Result<Application> create(const AppOptions&);
  Result<WindowHandle> create_window(const WindowOptions&);
  Result<void> post(std::function<void()> task);
  Result<int> run();
};

class WindowHandle {
public:
  Result<BridgeHandle> bridge();
  Result<void> set_title(std::string_view);
  Result<void> maximize();
  Result<void> unmaximize();
  Result<void> minimize();
  Result<void> unminimize();
  Result<void> show();
  Result<void> hide();
  Result<void> focus();
  Result<void> set_size(Size);
  Result<Size> get_size() const;
  Result<void> set_position(Position);
  Result<Position> get_position() const;
  Result<void> set_borderless(bool);
  Result<void> set_always_on_top(bool);
  Result<void> close();
  Result<void> load_url(std::string_view);
  Result<void> load_file(std::string_view);
  Result<void> reload();
  Result<void> evaluate_script(std::string_view);
  Result<void> add_init_script(std::string_view);
  Result<void> open_devtools();
  Result<void> close_devtools();
  Result<void> on_page_load(PageLoadHandler);
  Result<void> set_navigation_handler(NavigationHandler);
  Result<Capabilities> capabilities() const;
};

class BridgeHandle {
public:
  Result<void> register_command(std::string, CommandHandler);
  Result<void> emit(std::string, const Json&);
};

// `post(...)` is the only cross-thread-safe entry point.
// Other host calls return `wrong_thread` if called off the main loop thread.
// `BridgeHandle::register_command(...)` stores handlers on shared runtime state before `run()` starts.
// duplicate command names return `command_already_registered`.
// startup-time window configuration remains valid before `run()` starts.
// Methods that depend on a WebView backend (`load_url`, `load_file`, `reload`, `evaluate_script`,
// `add_init_script`, `open_devtools`, `close_devtools`, `on_page_load`, `set_navigation_handler`,
// and `capabilities`) are declaration-only in this task and return `unsupported_by_backend`
// until later chunks wire the real backend.
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/viewshell/window_handle.cpp src/viewshell/bridge_handle.cpp)

# update `tests/CMakeLists.txt`
add_executable(application_lifecycle_test unit/application_lifecycle_test.cpp)
target_link_libraries(application_lifecycle_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(application_lifecycle_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build build --target application_lifecycle_test && ctest --test-dir build -R ApplicationLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt include/viewshell/application.h include/viewshell/window_handle.h include/viewshell/bridge_handle.h src/viewshell/application.cpp src/viewshell/window_handle.cpp src/viewshell/bridge_handle.cpp tests/CMakeLists.txt tests/unit/application_lifecycle_test.cpp
git commit -m "feat: add public application window and bridge handles"
```

### Task 4: Enforce Single-Window Runtime And Stable Closed Handles

**Files:**
- Modify: `include/viewshell/application.h`
- Modify: `include/viewshell/window_handle.h`
- Modify: `include/viewshell/bridge_handle.h`
- Modify: `src/viewshell/application.cpp`
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `src/viewshell/bridge_handle.cpp`
- Create: `src/viewshell/runtime_state.h`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/unit/runtime_test_hooks.h`
- Create: `tests/unit/runtime_test_hooks.cpp`
- Create: `tests/unit/window_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(WindowLifecycle, second_window_is_rejected) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  auto second = app.create_window({});
  ASSERT_FALSE(second);
  EXPECT_EQ(second.error().code, "multiple_windows_unsupported");
}
```

```cpp
TEST(WindowLifecycle, closed_handle_reports_window_closed) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  MarkWindowClosedForTest(window);
  auto result = window.set_borderless(true);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "window_closed");
}
```

```cpp
TEST(WindowLifecycle, closed_bridge_handle_reports_window_closed) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  MarkWindowClosedForTest(window);
  auto result = bridge.emit("native-ready", nlohmann::json::object());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "window_closed");
}
```

```cpp
TEST(WindowLifecycle, closed_bridge_registration_reports_window_closed) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  MarkWindowClosedForTest(window);
  auto result = bridge.register_command("app.ping", [](const Json&) -> Result<Json> { return Json{{"ok", true}}; });
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "window_closed");
}
```

```cpp
TEST(WindowLifecycle, off_thread_host_calls_return_wrong_thread) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();

  std::thread worker([&] {
    auto result = window.set_borderless(true);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, "wrong_thread");
  });
  worker.join();
}
```

```cpp
TEST(WindowLifecycle, post_before_run_is_invalid_state) {
  auto app = viewshell::Application::create({}).value();
  auto result = app.post([] {});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_state");
}
```

```cpp
TEST(WindowLifecycle, post_after_run_started_executes_task) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  MarkRunStartedForTest(app);
  bool called = false;
  ASSERT_TRUE(app.post([&] { called = true; }));
  PumpPostedTasksForTest(app);
  EXPECT_TRUE(called);
}
```

```cpp
TEST(WindowLifecycle, post_from_worker_thread_after_run_started_succeeds) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  MarkRunStartedForTest(app);
  bool called = false;

  std::thread worker([&] {
    ASSERT_TRUE(app.post([&] { called = true; }));
  });
  worker.join();
  PumpPostedTasksForTest(app);
  EXPECT_TRUE(called);
}
```

```cpp
TEST(WindowLifecycle, post_after_shutdown_is_invalid_state) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  MarkShutdownStartedForTest(app);
  auto result = app.post([] {});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_state");
}
```

```cpp
TEST(WindowLifecycle, posted_exceptions_are_caught) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  MarkRunStartedForTest(app);
  bool still_running = false;
  ASSERT_TRUE(app.post([] { throw std::runtime_error("boom"); }));
  ASSERT_TRUE(app.post([&] { still_running = true; }));
  PumpPostedTasksForTest(app);
  EXPECT_FALSE(TakeRuntimeLogsForTest(app).empty());
  EXPECT_TRUE(still_running);
}
```

```cpp
TEST(WindowLifecycle, create_window_after_run_started_is_rejected_as_multiple_windows_unsupported) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  MarkRunStartedForTest(app);
  auto result = app.create_window({});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "multiple_windows_unsupported");
}
```

```cpp
TEST(WindowLifecycle, create_window_after_close_is_rejected_as_multiple_windows_unsupported) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  MarkWindowClosedForTest(window);
  auto result = app.create_window({});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "multiple_windows_unsupported");
}
```

```cpp
TEST(WindowLifecycle, close_starts_shutdown_and_run_exits) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  EnterRunLoopForTest(app);
  ArmCloseAcknowledgementForTest(window);
  ASSERT_TRUE(window.close());
  auto result = FinishRunForTest(app);
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), 0);
}
```

```cpp
TEST(WindowLifecycle, run_returns_exit_code_once_shutdown_started) {
  auto app = viewshell::Application::create({}).value();
  EXPECT_TRUE(app.create_window({}));
  MarkShutdownStartedForTest(app);
  auto result = app.run();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), 0);
}
```

```cpp
TEST(WindowLifecycle, emit_before_bridge_activation_is_unavailable) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  auto bridge = window.bridge().value();
  auto result = bridge.emit("native-ready", nlohmann::json::object());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "bridge_unavailable");
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build --target window_lifecycle_test && ctest --test-dir build -R WindowLifecycle --output-on-failure`
Expected: FAIL because stable close semantics are not implemented.

- [ ] **Step 3: Write shared runtime state and closed-handle implementation**

```cpp
struct RuntimeWindowState {
  bool has_window = false;
  bool is_closed = false;
};

// Application owns one optional RuntimeWindowState.
// WindowHandle and BridgeHandle keep a shared_ptr<RuntimeWindowState>.
// After close, operational methods return `window_closed`.
// `src/viewshell/runtime_state.h` is the single home for this shared state type.
// `MarkWindowClosedForTest(WindowHandle&)` is implemented at this step for the closed-handle subset.
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/viewshell/application.cpp src/viewshell/window_handle.cpp src/viewshell/bridge_handle.cpp)

# update `tests/CMakeLists.txt`
add_executable(window_lifecycle_test unit/window_lifecycle_test.cpp unit/runtime_test_hooks.cpp)
target_link_libraries(window_lifecycle_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(window_lifecycle_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run closed-handle subset tests**

Run: `cmake --build build --target window_lifecycle_test && ctest --test-dir build -R "WindowLifecycle\.(second_window_is_rejected|closed_handle_reports_window_closed|closed_bridge_handle_reports_window_closed|closed_bridge_registration_reports_window_closed|create_window_after_close_is_rejected_as_multiple_windows_unsupported)" --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Write run-loop, queue, and thread-affinity hooks**

```cpp
// `tests/unit/runtime_test_hooks.h` also provides:
// - `PumpPostedTasksForTest(Application&)`
// - `EnterRunLoopForTest(Application&)`
// - `FinishRunForTest(Application&)`
// - `MarkRunStartedForTest(Application&)`
// - `MarkShutdownStartedForTest(Application&)`
// - `ArmCloseAcknowledgementForTest(WindowHandle&)`
// `post(...)` returns `invalid_state` before `run()` starts.
// Host methods off the main loop thread return `wrong_thread`.
// When `run_started` is true, `post(...)` queues tasks and `PumpPostedTasksForTest(Application&)`
// advances the test-controlled main loop to execute them.
// `EnterRunLoopForTest(...)` marks the app as being inside the run loop without blocking.
// `FinishRunForTest(...)` drains shutdown and returns `run_exit_code` once shutdown has started.
// `close()` flips `shutdown_started`, waits for close acknowledgement, and lets `FinishRunForTest(...)` return `run_exit_code`.
// These hooks are test-only and compiled only into the unit-test target.
// Their implementation lives in `tests/unit/runtime_test_hooks.cpp` and uses friend/test access to runtime state.

// Expanded runtime state for this step:
// struct RuntimeAppState {
//   bool run_started;
//   bool shutdown_started;
//   int run_exit_code;
//   std::thread::id owner_thread;
//   std::deque<std::function<void()>> posted_tasks;
//   std::mutex mutex;
//   std::condition_variable cv;
//   std::vector<std::string> logs;
// };
// struct RuntimeWindowState {
//   bool has_window;
//   bool is_closed;
//   bool close_acknowledged;
//   WindowOptions pending_window_options;
//   std::unordered_map<std::string, CommandHandler> command_registry;
// };

// Caught `post(...)` exceptions are appended to `RuntimeAppState::logs`, and
// `TakeRuntimeLogsForTest(Application&)` reads from that buffer.

// Hook signatures:
// void MarkRunStartedForTest(Application&);
// void MarkShutdownStartedForTest(Application&);
// void MarkWindowClosedForTest(WindowHandle&);
// void ArmCloseAcknowledgementForTest(WindowHandle&);
// void EnterRunLoopForTest(Application&);
// Result<int> FinishRunForTest(Application&);
// void PumpPostedTasksForTest(Application&);
// std::vector<std::string> TakeRuntimeLogsForTest(Application&);
```

- [ ] **Step 6: Run full lifecycle suite**

Run: `cmake --build build --target window_lifecycle_test && ctest --test-dir build -R WindowLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt include/viewshell/application.h include/viewshell/window_handle.h include/viewshell/bridge_handle.h src/viewshell/application.cpp src/viewshell/window_handle.cpp src/viewshell/bridge_handle.cpp src/viewshell/runtime_state.h tests/CMakeLists.txt tests/unit/runtime_test_hooks.h tests/unit/runtime_test_hooks.cpp tests/unit/window_lifecycle_test.cpp
git commit -m "feat: enforce single-window lifecycle and closed-handle behavior"
```

## Chunk 2: X11 Backend, Resolver, And Resource Protocol

### Task 5: Add X11 WindowDriver Foundation

**Files:**
- Create: `src/platform/linux_x11/window_driver.h`
- Create: `src/platform/linux_x11/window_driver.cpp`
- Create: `src/platform/linux_x11/drag_context.h`
- Create: `src/platform/linux_x11/native_window_handle.h`
- Modify: `CMakeLists.txt`
- Modify: `cmake/ViewshellDeps.cmake`
- Modify: `tests/CMakeLists.txt`
- Modify: `src/viewshell/application.cpp`
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Create: `tests/integration/window_driver_test_hooks.h`
- Create: `tests/integration/window_driver_smoke_test.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST(WindowDriver, applies_basic_window_operations) {
  viewshell::WindowOptions options;
  options.width = 640;
  options.height = 480;
  options.borderless = true;

  WindowDriver driver;
  ASSERT_TRUE(driver.create(options));
  EXPECT_TRUE(driver.set_title("Viewshell"));
  EXPECT_TRUE(driver.set_size({800, 600}));
  EXPECT_TRUE(driver.set_position({20, 30}));
  EXPECT_TRUE(driver.get_position());
  EXPECT_TRUE(driver.get_size());
  EXPECT_TRUE(driver.set_borderless(false));
  EXPECT_TRUE(driver.set_always_on_top(true));
  EXPECT_TRUE(driver.maximize());
  EXPECT_TRUE(driver.unmaximize());
  EXPECT_TRUE(driver.minimize());
  EXPECT_TRUE(driver.unminimize());
  EXPECT_TRUE(driver.show());
  EXPECT_TRUE(driver.hide());
  EXPECT_TRUE(driver.focus());
  EXPECT_TRUE(driver.close());
}
```

```cpp
TEST(WindowHandleForwarding, host_window_handle_forwards_to_driver) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  EXPECT_TRUE(window.set_size({800, 600}));
  EXPECT_TRUE(window.set_borderless(true));
  auto size = window.get_size();
  ASSERT_TRUE(size);
  EXPECT_EQ(size->width, 800);
  EXPECT_EQ(size->height, 600);
}
```

```cpp
TEST(WindowDriver, rejects_begin_drag_without_active_drag_context) {
  WindowDriver driver;
  ASSERT_TRUE(driver.create({}));
  DragContext drag{};
  drag.is_valid = false;
  auto result = driver.begin_drag(drag);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_drag_context");
}
```

```cpp
TEST(WindowDriver, emits_resize_focus_and_close_callbacks) {
  WindowDriver driver;
  bool closed = false;
  bool focused = false;
  bool resized = false;
  driver.on_close = [&] { closed = true; };
  driver.on_focus = [&](bool value) { focused = value; };
  driver.on_resize = [&](Size) { resized = true; };
  ASSERT_TRUE(driver.create({}));
  TriggerWindowCallbacksForTest(driver);
  EXPECT_TRUE(closed);
  EXPECT_TRUE(focused);
  EXPECT_TRUE(resized);
}
```

```cpp
TEST(WindowDriver, close_callback_drives_runtime_shutdown) {
  auto app = viewshell::Application::create({}).value();
  auto window = app.create_window({}).value();
  EnterRunLoopForTest(app);
  TriggerCloseCallbackForTest(window);
  auto result = FinishRunForTest(app);
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), 0);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target window_driver_smoke_test && xvfb-run -a ctest --test-dir build -R "WindowDriver|WindowHandleForwarding" --output-on-failure`
Expected: FAIL because no GTK/X11 driver exists.

- [ ] **Step 3: Write minimal implementation**

```cpp
class WindowDriver {
public:
  Result<NativeWindowHandle> create(const WindowOptions&);
  Result<void> set_title(std::string_view);
  Result<void> set_size(Size);
  Result<Size> get_size() const;
  Result<void> set_position(Position);
  Result<Position> get_position() const;
  Result<void> set_borderless(bool);
  Result<void> set_always_on_top(bool);
  Result<void> maximize();
  Result<void> unmaximize();
  Result<void> minimize();
  Result<void> unminimize();
  Result<void> show();
  Result<void> hide();
  Result<void> focus();
  Result<void> begin_drag(DragContext);
  Result<void> close();

  std::function<void()> on_close;
  std::function<void(Size)> on_resize;
  std::function<void(bool)> on_focus;
};

// `src/platform/linux_x11/native_window_handle.h` defines the attach target type.
// `WindowDriver` owns the handle lifetime and `WebviewDriver` borrows it during attach.
// `tests/integration/window_driver_test_hooks.h` exposes `TriggerWindowCallbacksForTest(WindowDriver&)`.
// That same hook header also exposes `TriggerCloseCallbackForTest(WindowHandle&)`
// for runtime shutdown wiring tests.

// Backend-shared types for this step:
// struct NativeWindowHandle {
//   GtkWidget* gtk_window;
//   void* x11_display;
//   unsigned long x11_window;
// };
// struct DragContext {
//   bool is_valid;
//   int button;
//   int root_x;
//   int root_y;
//   unsigned int timestamp;
// };

// `cmake/ViewshellDeps.cmake` adds GTK3 and X11 pkg-config include/link flags to `viewshell`.
```

```cmake
# update `cmake/ViewshellDeps.cmake`
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(X11 REQUIRED x11)
target_include_directories(viewshell PRIVATE ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
target_link_libraries(viewshell PRIVATE ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_options(viewshell PRIVATE ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/window_driver.cpp)

# update `tests/CMakeLists.txt`
add_executable(window_driver_smoke_test integration/window_driver_smoke_test.cpp)
target_link_libraries(window_driver_smoke_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(window_driver_smoke_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Implement raw WindowDriver operations and callback surfaces**

```cpp
// Implement in `src/platform/linux_x11/window_driver.cpp`:
// - create GTK window and X11 handle pair
// - bind GTK destroy/configure/focus events to `on_close/on_resize/on_focus`
// - implement `set_title`, size/position getters/setters, show/hide, focus,
//   maximize/minimize/unmaximize/unminimize, always-on-top, borderless, close
// - return `invalid_drag_context` when `begin_drag(DragContext)` receives `is_valid == false`
```

- [ ] **Step 5: Hook runtime ownership and WindowHandle forwarding to the driver**

```cpp
// Implement in `src/viewshell/application.cpp` and `src/viewshell/window_handle.cpp`:
// - create and store one active `WindowDriver` in runtime/app state
// - store the borrowed `NativeWindowHandle` returned by `WindowDriver::create(...)`
// - wire `WindowDriver::on_close` to begin shutdown and set `run_exit_code`
// - forward `WindowHandle` public methods to the active driver
// - keep `WindowHandle` forwarding returning `window_closed` after close
```

- [ ] **Step 6: Run test to verify it passes**

Run: `cmake --build build --target window_driver_smoke_test window_lifecycle_test application_lifecycle_test && xvfb-run -a ctest --test-dir build -R "WindowDriver|WindowHandleForwarding|WindowLifecycle|ApplicationLifecycle" --output-on-failure`
Expected: PASS for the native driver, host-handle forwarding, and previously locked lifecycle behavior under the Linux X11 CI/runtime environment.

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt cmake/ViewshellDeps.cmake tests/CMakeLists.txt src/platform/linux_x11/drag_context.h src/platform/linux_x11/native_window_handle.h src/platform/linux_x11/window_driver.h src/platform/linux_x11/window_driver.cpp src/viewshell/application.cpp src/viewshell/window_handle.cpp src/viewshell/runtime_state.h tests/integration/window_driver_test_hooks.h tests/integration/window_driver_smoke_test.cpp
git commit -m "feat: add x11 window driver foundation"
```

### Task 6: Add Deterministic KernelResolver

**Files:**
- Create: `src/platform/linux_x11/kernel_resolver.h`
- Create: `src/platform/linux_x11/kernel_resolver.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/unit/kernel_resolver_test.cpp`
- Create: `tests/unit/kernel_resolver_test_support.h`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(KernelResolver, rejects_non_webkit_required_engine) {
  AppOptions options;
  options.require_engine = "webview2";
  auto result = KernelResolver::resolve(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "engine_incompatible");
}
```

```cpp
TEST(KernelResolver, uses_explicit_engine_path_first) {
  AppOptions options;
  options.engine_path = "/tmp/libwebkit-custom.so";
  auto result = KernelResolver::candidate_paths(options);
  ASSERT_EQ(result.front(), "/tmp/libwebkit-custom.so");
}
```

```cpp
TEST(KernelResolver, reports_not_found_with_attempted_candidates) {
  auto result = ResolveWithProbeForTest(MissingOnlyProbe{});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "engine_not_found");
  EXPECT_FALSE(result.error().details["attempted_candidates"].empty());
  EXPECT_FALSE(result.error().details["failure_reasons"].empty());
}
```

```cpp
TEST(KernelResolver, reports_engine_init_failed_when_probe_creation_fails) {
  auto result = ResolveWithProbeForTest(FailingInitProbe{});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "engine_init_failed");
}
```

```cpp
TEST(KernelResolver, reduces_optional_capabilities_when_devtools_missing) {
  auto result = ResolveWithProbeForTest(NoDevtoolsProbe{});
  ASSERT_TRUE(result);
  EXPECT_FALSE(result->capabilities.webview.devtools);
}
```

```cpp
TEST(KernelResolver, falls_back_to_second_candidate_after_first_required_probe_failure) {
  auto result = ResolveWithProbeForTest(FallbackSuccessProbe{});
  ASSERT_TRUE(result);
  EXPECT_EQ(result->library_path, "libwebkit2gtk-4.0.so.37");
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build --target kernel_resolver_test && ctest --test-dir build -R KernelResolver --output-on-failure`
Expected: FAIL because resolver logic does not exist.

- [ ] **Step 3: Write minimal implementation**

```cpp
struct ResolvedEngine {
  std::string engine_id;
  std::string library_path;
  Capabilities capabilities;
};

class KernelResolver {
public:
  static Result<ResolvedEngine> resolve(const AppOptions&);
  static std::vector<std::string> candidate_paths(const AppOptions&);
};

struct TestProbe {
  std::vector<std::string> candidate_paths;
  std::function<Result<ResolvedEngine>(const AppOptions&, std::string_view candidate_path)> resolve_candidate;
};

// Candidate order:
// 1. AppOptions.engine_path
// 2. libwebkit2gtk-4.1.so.0
// 3. libwebkit2gtk-4.0.so.37

// Required probe set for candidate acceptance:
// - webview creation
// - page loading
// - script execution
// - script-message bridge
// - init-script injection
// Optional probe set for reduced-capability acceptance:
// - devtools
// - extended diagnostics

// Resolver records attempted candidates and failure reasons,
// and distinguishes engine_incompatible / engine_not_found / engine_init_failed.
// `tests/unit/kernel_resolver_test_support.h` exposes `ResolveWithProbeForTest(TestProbe)`.
// `MissingOnlyProbe`, `FailingInitProbe`, `NoDevtoolsProbe`, and `FallbackSuccessProbe`
// live in `tests/unit/kernel_resolver_test.cpp` and are adapted into `TestProbe`
// instances to make candidate-by-candidate fallback and error reporting deterministic.
// `error.details` includes both `attempted_candidates` and `failure_reasons` arrays.
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/kernel_resolver.cpp)

# update `tests/CMakeLists.txt`
add_executable(kernel_resolver_test unit/kernel_resolver_test.cpp)
target_link_libraries(kernel_resolver_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(kernel_resolver_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build build --target kernel_resolver_test && ctest --test-dir build -R KernelResolver --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt src/platform/linux_x11/kernel_resolver.h src/platform/linux_x11/kernel_resolver.cpp tests/unit/kernel_resolver_test_support.h tests/unit/kernel_resolver_test.cpp
git commit -m "feat: add deterministic webkitgtk resolver"
```

### Task 7: Add ResourceProtocol For `viewshell://app/`

**Files:**
- Create: `src/platform/linux_x11/resource_protocol.h`
- Create: `src/platform/linux_x11/resource_protocol.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/unit/resource_protocol_test.cpp`
- Create: `tests/fixtures/local_app/index.html`
- Create: `tests/fixtures/local_app/main.js`
- Create: `tests/fixtures/local_app/blob.unknownext`
- Create: `tests/fixtures/alt_app/index.html`
- Create: `tests/fixtures/local_app/settings/index.html`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(ResourceProtocol, rejects_paths_outside_asset_root) {
  ResourceProtocol protocol("/tmp/app");
  auto result = protocol.resolve("viewshell://app/../secret.txt");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}
```

```cpp
TEST(ResourceProtocol, maps_missing_file_to_resource_not_found) {
  ResourceProtocol protocol("/tmp/app");
  auto result = protocol.resolve("viewshell://app/missing.js");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_not_found");
}
```

```cpp
TEST(ResourceProtocol, missing_entry_file_maps_to_resource_not_found) {
  auto result = ResourceProtocol::from_entry_file("tests/fixtures/local_app/missing.html", {});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_not_found");
}
```

```cpp
TEST(ResourceProtocol, falls_back_to_application_octet_stream_for_unknown_extension) {
  ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("viewshell://app/blob.unknownext");
  ASSERT_TRUE(result);
  EXPECT_EQ(result->mime_type, "application/octet-stream");
}
```

```cpp
TEST(ResourceProtocol, rejects_wrong_scheme) {
  ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("file://app/index.html");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}
```

```cpp
TEST(ResourceProtocol, rejects_wrong_host) {
  ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("viewshell://other/index.html");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}
```

```cpp
TEST(ResourceProtocol, resolves_index_under_viewshell_app_origin) {
  ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("viewshell://app/index.html");
  ASSERT_TRUE(result);
  EXPECT_EQ(result->mime_type, "text/html");
}
```

```cpp
TEST(ResourceProtocol, rejects_entry_outside_explicit_asset_root) {
  viewshell::WindowOptions options;
  options.asset_root = "/tmp/app";
  auto result = ResourceProtocol::from_entry_file("/tmp/other/index.html", options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}
```

```cpp
TEST(ResourceProtocol, reestablishes_inferred_scope_for_new_entry_file) {
  auto first = ResourceProtocol::from_entry_file("tests/fixtures/local_app/index.html", {});
  auto second = ResourceProtocol::from_entry_file("tests/fixtures/alt_app/index.html", {});
  ASSERT_TRUE(first);
  ASSERT_TRUE(second);
  EXPECT_NE(first->asset_root(), second->asset_root());
}
```

```cpp
TEST(ResourceProtocol, preserves_nested_entry_path_under_viewshell_app_origin) {
  auto nested = ResourceProtocol::from_entry_file("tests/fixtures/local_app/settings/index.html", {});
  ASSERT_TRUE(nested);
  EXPECT_EQ(nested->entry_url(), "viewshell://app/settings/index.html");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target resource_protocol_test && ctest --test-dir build -R ResourceProtocol --output-on-failure`
Expected: FAIL because the app protocol and fixture app do not exist.

- [ ] **Step 3: Write minimal implementation**

```cpp
class ResourceProtocol {
public:
  explicit ResourceProtocol(std::filesystem::path asset_root);
  static Result<ResourceProtocol> from_entry_file(std::string_view entry_file, const WindowOptions& options);
  Result<ResourceResponse> resolve(std::string_view url);
  const std::filesystem::path& asset_root() const;
  std::string entry_url() const;
};

struct ResourceResponse {
  std::string mime_type;
  std::string body;
};

// Resolution rules:
// - URL scheme must be exactly `viewshell://app/...`
// - host must be exactly `app`
// - request path is canonicalized relative to `asset_root`
// - canonicalized path escaping `asset_root` returns `resource_out_of_scope`
// - unknown file extension maps to `application/octet-stream`
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/resource_protocol.cpp)

# update `tests/CMakeLists.txt`
add_executable(resource_protocol_test unit/resource_protocol_test.cpp)
target_link_libraries(resource_protocol_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(resource_protocol_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build --target resource_protocol_test && ctest --test-dir build -R ResourceProtocol --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt src/platform/linux_x11/resource_protocol.h src/platform/linux_x11/resource_protocol.cpp tests/unit/resource_protocol_test.cpp tests/fixtures/local_app/index.html tests/fixtures/local_app/main.js tests/fixtures/local_app/blob.unknownext tests/fixtures/local_app/settings/index.html tests/fixtures/alt_app/index.html
git commit -m "feat: add local viewshell resource protocol"
```

## Chunk 3: WebKitGTK Attachment And Trusted Local App

### Task 8: Attach WebKitGTK WebView And Validate Trusted Local App

**Files:**
- Create: `src/platform/linux_x11/webview_driver.h`
- Create: `src/platform/linux_x11/webview_driver.cpp`
- Modify: `CMakeLists.txt`
- Modify: `cmake/ViewshellDeps.cmake`
- Modify: `tests/CMakeLists.txt`
- Modify: `src/viewshell/application.cpp`
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Create: `tests/integration/test_app_harness.h`
- Create: `tests/integration/test_app_harness.cpp`
- Create: `tests/integration/local_app_smoke_test.cpp`
- Create: `tests/unit/webview_driver_test.cpp`
- Modify: `tests/fixtures/local_app/index.html`
- Modify: `tests/fixtures/local_app/main.js`
- Modify: `tests/fixtures/alt_app/index.html`
- Create: `tests/fixtures/local_app/subframe.html`

- [ ] **Step 1: Write the failing test**

```cpp
TEST(LocalAppSmoke, serves_index_under_viewshell_app_origin) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_EQ(app.eval_js("location.origin"), "viewshell://app");
}
```

```cpp
TEST(LocalAppSmoke, injects_init_scripts_only_into_trusted_top_level_document) {
  TestAppHarness app;
  app.add_init_script("window.__init_order = ['init'];");
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_EQ(app.eval_js("document.querySelector('iframe')?.contentWindow?.Viewshell"), "undefined");
  ASSERT_EQ(app.eval_js("document.querySelector('iframe')?.contentWindow?.__init_order"), "undefined");
}
```

```cpp
TEST(WebviewDriver, rejects_unsupported_url_schemes) {
  WebviewDriver driver;
  ASSERT_FALSE(driver.load_url("file:///tmp/index.html"));
  ASSERT_FALSE(driver.load_url("viewshell://app/index.html"));
  EXPECT_EQ(driver.load_url("file:///tmp/index.html").error().code, "unsupported_url_scheme");
}
```

```cpp
TEST(WebviewDriver, emits_page_load_and_honors_navigation_denial) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_EQ(app.last_page_load_stage(), "finished");
  app.set_navigation_decision(viewshell::NavigationDecision::Deny);
  auto result = app.navigate("https://example.com");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "navigation_denied");
}
```

```cpp
TEST(WebviewDriver, denied_navigation_emits_no_page_load_callback) {
  TestAppHarness app;
  int calls = 0;
  app.on_page_load([&](const viewshell::PageLoadEvent&) { calls += 1; });
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  calls = 0;
  app.set_navigation_decision(viewshell::NavigationDecision::Deny);
  ASSERT_FALSE(app.navigate("https://example.com"));
  EXPECT_EQ(calls, 0);
}
```

```cpp
TEST(WebviewDriver, runs_init_scripts_before_page_scripts) {
  TestAppHarness app;
  app.add_init_script("window.__init_order = ['init'];");
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_EQ(app.eval_js("window.__init_order.join(',')"), "init,page");
}
```

```cpp
TEST(WebviewDriver, reports_navigation_superseded_and_late_failures) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_TRUE(app.trigger_navigation_superseded_for_test());
  EXPECT_EQ(app.last_page_load_error(), "navigation_superseded");
}
```

```cpp
TEST(WebviewDriver, reports_late_navigation_failure) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_TRUE(app.trigger_navigation_failure_for_test());
  EXPECT_EQ(app.last_page_load_error(), "navigation_failed");
}
```

```cpp
TEST(WebviewDriver, catches_callback_exceptions_and_continues) {
  TestAppHarness app;
  app.on_page_load([](const viewshell::PageLoadEvent&) { throw std::runtime_error("boom"); });
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  EXPECT_FALSE(app.take_logs().empty());
  ASSERT_EQ(app.last_page_load_stage(), "finished");
}
```

```cpp
TEST(WebviewDriver, treats_throwing_navigation_handler_as_deny) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  app.on_navigation([](const viewshell::NavigationRequest&) -> viewshell::NavigationDecision {
    throw std::runtime_error("boom");
  });
  auto result = app.navigate("https://example.com");
  ASSERT_FALSE(result);
  EXPECT_FALSE(app.take_logs().empty());
  EXPECT_EQ(result.error().code, "navigation_denied");
}
```

```cpp
TEST(WebviewDriver, successful_navigation_emits_started_then_finished) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  EXPECT_EQ(app.page_load_stages(), std::vector<std::string>({"started", "finished"}));
}
```

```cpp
TEST(WebviewDriver, accumulates_page_load_handlers_and_replaces_navigation_handler) {
  TestAppHarness app;
  int calls = 0;
  app.on_page_load([&](const viewshell::PageLoadEvent&) { calls += 1; });
  app.on_page_load([&](const viewshell::PageLoadEvent&) { calls += 1; });
  app.on_navigation([](const viewshell::NavigationRequest&) { return viewshell::NavigationDecision::Allow; });
  app.on_navigation([](const viewshell::NavigationRequest&) { return viewshell::NavigationDecision::Deny; });
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  EXPECT_GE(calls, 2);
  auto result = app.navigate("https://example.com");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "navigation_denied");
}
```

```cpp
TEST(WebviewDriver, exposes_host_webview_controls_and_raw_capabilities) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  auto caps = app.host_capabilities();
  ASSERT_TRUE(caps);
  EXPECT_TRUE(caps->window.native_drag);
  EXPECT_TRUE(caps->webview.resource_protocol);
  EXPECT_TRUE(caps->bridge.invoke);
  EXPECT_TRUE(app.host_window().set_title("Viewshell"));
  EXPECT_TRUE(app.host_window().reload());
  EXPECT_TRUE(app.host_window().evaluate_script("void 0"));
  if(!caps->webview.devtools) {
    auto open = app.host_window().open_devtools();
    ASSERT_FALSE(open);
    EXPECT_EQ(open.error().code, "unsupported_by_backend");
  }
}
```

```cpp
TEST(LocalAppSmoke, second_load_file_reestablishes_active_asset_scope) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_TRUE(app.host_window().load_file("tests/fixtures/alt_app/index.html"));
  ASSERT_EQ(app.eval_js("document.body.dataset.app"), "alt");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target webview_driver_test local_app_smoke_test && xvfb-run -a ctest --test-dir build -R "WebviewDriver|LocalAppSmoke" --output-on-failure`
Expected: FAIL because the WebKitGTK attachment path and app protocol hookup do not exist.

- [ ] **Step 3: Write minimal implementation**

```cpp
class WebviewDriver {
public:
  Result<void> attach(NativeWindowHandle, const WindowOptions&, const ResolvedEngine&);
  Result<void> load_file(std::string_view entry_file);
  Result<void> load_url(std::string_view url);
  Result<void> reload();
  Result<void> evaluate_script(std::string_view script);
  Result<void> add_init_script(std::string_view script);
  Result<void> open_devtools();
  Result<void> close_devtools();
  Result<void> on_page_load(PageLoadHandler handler);
  Result<void> set_navigation_handler(NavigationHandler handler);

  // lifecycle state stored in `src/viewshell/runtime_state.h` includes:
  // - active ResolvedEngine
  // - active inferred asset scope
  // - accumulated PageLoadHandler list
  // - replaceable NavigationHandler
  // - persistent init script list
};

class TestAppHarness {
public:
  bool launch_local(const std::string& entry_file);
  bool launch_remote(const std::string& url);
  std::string eval_js(const std::string& expression);
  std::string eval_async_js(const std::string& expression);
  std::string last_page_load_stage() const;
  std::string last_page_load_error() const;
  std::vector<std::string> page_load_stages() const;
  void set_navigation_decision(viewshell::NavigationDecision decision);
  viewshell::Result<void> navigate(const std::string& url);
  bool navigate_async(const std::string& url);
  bool trigger_navigation_superseded_for_test();
  bool trigger_navigation_failure_for_test();
  void add_init_script(const std::string& script);
  void on_page_load(viewshell::PageLoadHandler handler);
  void on_navigation(viewshell::NavigationHandler handler);
  std::vector<std::string> take_logs();
  viewshell::Result<viewshell::Capabilities> host_capabilities() const;
  viewshell::WindowHandle host_window() const;
};

// `src/viewshell/application.cpp` calls `KernelResolver::resolve(...)`,
// stores the accepted `ResolvedEngine`, and passes it into `WebviewDriver::attach(...)`.
// `tests/fixtures/local_app/index.html` creates a real iframe that loads `subframe.html`.
// `tests/fixtures/local_app/main.js` appends `"page"` to `window.__init_order`.
// `tests/fixtures/alt_app/index.html` sets `document.body.dataset.app = "alt"`.
// `src/runtime/js/viewshell_init.js` is embedded into a generated C++ string constant and
// injected only into trusted top-level pages.
```

```cmake
# update `cmake/ViewshellDeps.cmake`
pkg_check_modules(WEBKITGTK41 QUIET webkit2gtk-4.1)
if(WEBKITGTK41_FOUND)
  target_include_directories(viewshell PRIVATE ${WEBKITGTK41_INCLUDE_DIRS})
  target_link_libraries(viewshell PRIVATE ${WEBKITGTK41_LIBRARIES})
  target_compile_options(viewshell PRIVATE ${WEBKITGTK41_CFLAGS_OTHER})
else()
  pkg_check_modules(WEBKITGTK40 REQUIRED webkit2gtk-4.0)
  target_include_directories(viewshell PRIVATE ${WEBKITGTK40_INCLUDE_DIRS})
  target_link_libraries(viewshell PRIVATE ${WEBKITGTK40_LIBRARIES})
  target_compile_options(viewshell PRIVATE ${WEBKITGTK40_CFLAGS_OTHER})
endif()

# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/webview_driver.cpp)

# update `tests/CMakeLists.txt`
add_executable(webview_driver_test unit/webview_driver_test.cpp)
target_link_libraries(webview_driver_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
target_sources(webview_driver_test PRIVATE integration/test_app_harness.cpp)
gtest_discover_tests(webview_driver_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(local_app_smoke_test integration/local_app_smoke_test.cpp)
target_link_libraries(local_app_smoke_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
target_sources(local_app_smoke_test PRIVATE integration/test_app_harness.cpp)
gtest_discover_tests(local_app_smoke_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build --target webview_driver_test local_app_smoke_test && xvfb-run -a ctest --test-dir build -R "WebviewDriver|LocalAppSmoke" --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt cmake/ViewshellDeps.cmake tests/CMakeLists.txt src/platform/linux_x11/webview_driver.h src/platform/linux_x11/webview_driver.cpp src/viewshell/application.cpp src/viewshell/window_handle.cpp src/viewshell/runtime_state.h tests/integration/test_app_harness.h tests/integration/test_app_harness.cpp tests/integration/local_app_smoke_test.cpp tests/unit/webview_driver_test.cpp tests/fixtures/local_app/index.html tests/fixtures/local_app/main.js tests/fixtures/local_app/subframe.html tests/fixtures/alt_app/index.html
git commit -m "feat: attach webkitgtk webview to trusted local app"
```

### Task 9: Add BridgeDriver Bootstrap And Integration Harness

**Files:**
- Create: `src/platform/linux_x11/bridge_driver.h`
- Create: `src/platform/linux_x11/bridge_driver.cpp`
- Create: `src/runtime/js/viewshell_init.js`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Modify: `src/viewshell/application.cpp`
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Create: `tests/unit/bridge_driver_test.cpp`
- Create: `tests/unit/bridge_driver_test_hooks.h`
- Modify: `tests/integration/test_app_harness.h`
- Modify: `tests/integration/test_app_harness.cpp`
- Modify: `tests/integration/local_app_smoke_test.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(BridgeDriver, signals_ready_reset_and_raw_message) {
  BridgeDriver driver;
  bool ready = false;
  bool reset = false;
  std::string raw;
  driver.on_bridge_ready = [&] { ready = true; };
  driver.on_bridge_reset = [&] { reset = true; };
  driver.on_raw_message = [&](std::string_view msg) { raw = msg; };
  ASSERT_TRUE(TriggerBridgeReadyForTest(driver));
  ASSERT_TRUE(TriggerBridgeRawMessageForTest(driver, "hello"));
  ASSERT_TRUE(TriggerBridgeResetForTest(driver));
  EXPECT_TRUE(ready);
  EXPECT_TRUE(reset);
  EXPECT_EQ(raw, "hello");
}
```

```cpp
TEST(LocalAppSmoke, trusted_top_level_page_bootstraps_viewshell) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_EQ(app.eval_js("typeof window.Viewshell"), "object");
}
```

```cpp
TEST(LocalAppSmoke, bootstrap_runs_before_user_init_and_page_scripts) {
  TestAppHarness app;
  app.add_init_script("window.__init_order.push('init');");
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  ASSERT_EQ(app.eval_js("window.__init_order.join(',')"), "bootstrap,init,page");
}
```

```cpp
TEST(BridgeDriver, post_to_page_delivers_to_active_generation_only) {
  BridgeDriver driver;
  ASSERT_FALSE(driver.post_to_page("before-ready"));
  ASSERT_TRUE(TriggerBridgeReadyForTest(driver));
  ASSERT_TRUE(driver.post_to_page("ping"));
  ASSERT_EQ(LastPostedMessageForTest(driver), "ping");
  ASSERT_TRUE(TriggerBridgeResetForTest(driver));
  ASSERT_FALSE(driver.post_to_page("after-reset"));
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build --target bridge_driver_test local_app_smoke_test && xvfb-run -a ctest --test-dir build -R "BridgeDriver|LocalAppSmoke" --output-on-failure`
Expected: FAIL because bridge bootstrap and harness wiring do not exist.

- [ ] **Step 3: Write minimal implementation**

```cpp
class BridgeDriver {
public:
  Result<void> attach(WebviewDriver&);
  Result<void> post_to_page(std::string_view raw_message);

  std::function<void()> on_bridge_ready;
  std::function<void()> on_bridge_reset;
  std::function<void(std::string_view)> on_raw_message;
};

class TestAppHarness {
public:
  bool launch_local(const std::string& entry_file);
  std::string eval_js(const std::string& expression);
  std::string last_page_load_stage() const;
  std::string last_page_load_error() const;
  std::vector<std::string> page_load_stages() const;
  void set_navigation_decision(viewshell::NavigationDecision decision);
  viewshell::Result<void> navigate(const std::string& url);
  bool navigate_async(const std::string& url);
  void add_init_script(const std::string& script);
  void on_page_load(viewshell::PageLoadHandler handler);
  void on_navigation(viewshell::NavigationHandler handler);
  std::vector<std::string> take_logs();
  viewshell::Result<viewshell::Capabilities> host_capabilities() const;
  viewshell::WindowHandle host_window() const;
};

// `src/runtime/js/viewshell_init.js` bootstraps `window.Viewshell` for trusted top-level pages only.
// It initializes `window.__init_order = ['bootstrap']` before user init scripts run.
// `tests/unit/bridge_driver_test_hooks.h` exposes `TriggerBridgeReadyForTest(...)`,
// `TriggerBridgeRawMessageForTest(...)`, `TriggerBridgeResetForTest(BridgeDriver&)`,
// and `LastPostedMessageForTest(BridgeDriver&)`.
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/bridge_driver.cpp)

# update `tests/CMakeLists.txt`
add_executable(bridge_driver_test unit/bridge_driver_test.cpp)
target_link_libraries(bridge_driver_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(bridge_driver_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build build --target bridge_driver_test local_app_smoke_test && xvfb-run -a ctest --test-dir build -R "BridgeDriver|LocalAppSmoke" --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt src/platform/linux_x11/bridge_driver.h src/platform/linux_x11/bridge_driver.cpp src/runtime/js/viewshell_init.js src/viewshell/application.cpp src/viewshell/window_handle.cpp src/viewshell/runtime_state.h tests/unit/bridge_driver_test.cpp tests/unit/bridge_driver_test_hooks.h tests/integration/test_app_harness.h tests/integration/test_app_harness.cpp tests/integration/local_app_smoke_test.cpp
git commit -m "feat: bootstrap trusted top-level bridge and harness"
```

## Chunk 4: Bridge, Trust Gate, And Verification

### Task 5: Implement TrustGate And Effective Capabilities

**Files:**
- Create: `src/platform/linux_x11/trust_gate.h`
- Create: `src/platform/linux_x11/trust_gate.cpp`
- Modify: `src/platform/linux_x11/webview_driver.cpp`
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/unit/trust_gate_test.cpp`
- Create: `tests/integration/remote_untrusted_test.cpp`
- Create: `tests/integration/remote_trusted_test.cpp`
- Modify: `tests/integration/test_app_harness.h`
- Modify: `tests/integration/test_app_harness.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(TrustGate, trusted_remote_gets_reduced_surface) {
  auto state = TrustGate::classify("https://allowed.example", {"https://allowed.example"});
  EXPECT_EQ(state.mode, TrustMode::ReducedBridge);
}
```

```cpp
TEST(RemoteUntrusted, bridge_not_injected) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_remote("https://untrusted.example"));
  ASSERT_EQ(app.eval_js("typeof window.Viewshell"), "undefined");
}
```

```cpp
TEST(RemoteTrusted, reduced_bridge_masks_webview_surface) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_remote("https://allowed.example"));
  ASSERT_EQ(app.eval_js("typeof window.Viewshell"), "object");
  ASSERT_EQ(app.eval_async_js("Viewshell.capabilities().then(c => String(c.webview.resource_protocol))"), "false");
  ASSERT_EQ(app.eval_async_js("Viewshell.webview.loadFile('/index.html').catch(e => e.code)"), "operation_not_allowed");
}
```

```cpp
TEST(RemoteTrusted, trust_follows_final_committed_origin_after_redirect) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_remote_with_redirect("https://redirect.example", "https://allowed.example"));
  ASSERT_EQ(app.eval_js("typeof window.Viewshell"), "object");
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build --target trust_gate_test remote_untrusted_test remote_trusted_test && xvfb-run -a ctest --test-dir build -R "TrustGate|RemoteUntrusted|RemoteTrusted" --output-on-failure`
Expected: FAIL because trust classification and remote gating are absent.

- [ ] **Step 3: Write minimal implementation**

```cpp
enum class TrustMode { NoBridge, ReducedBridge, FullBridge };

struct TrustDecision {
  TrustMode mode;
  Capabilities effective_capabilities;
};

// `tests/integration/test_app_harness.*` adds `eval_async_js(...)`
// for Promise-returning frontend assertions in this task, plus
// `launch_remote_with_redirect(...)` for final-committed-origin trust checks.
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/trust_gate.cpp)

# update `tests/CMakeLists.txt`
add_executable(trust_gate_test unit/trust_gate_test.cpp)
target_link_libraries(trust_gate_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(trust_gate_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(remote_untrusted_test integration/remote_untrusted_test.cpp)
target_link_libraries(remote_untrusted_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
target_sources(remote_untrusted_test PRIVATE integration/test_app_harness.cpp)
gtest_discover_tests(remote_untrusted_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(remote_trusted_test integration/remote_trusted_test.cpp)
target_link_libraries(remote_trusted_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
target_sources(remote_trusted_test PRIVATE integration/test_app_harness.cpp)
gtest_discover_tests(remote_trusted_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build build --target trust_gate_test remote_untrusted_test remote_trusted_test && xvfb-run -a ctest --test-dir build -R "TrustGate|RemoteUntrusted|RemoteTrusted" --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt src/platform/linux_x11/trust_gate.h src/platform/linux_x11/trust_gate.cpp src/platform/linux_x11/webview_driver.cpp src/viewshell/window_handle.cpp tests/unit/trust_gate_test.cpp tests/integration/test_app_harness.h tests/integration/test_app_harness.cpp tests/integration/remote_untrusted_test.cpp tests/integration/remote_trusted_test.cpp
git commit -m "feat: gate bridge access by trust context"
```

### Task 6: Add RequestTracker And Built-In Fast Path

**Files:**
- Create: `src/platform/linux_x11/request_tracker.h`
- Create: `src/platform/linux_x11/request_tracker.cpp`
- Create: `src/platform/linux_x11/builtin_dispatcher.h`
- Create: `src/platform/linux_x11/builtin_dispatcher.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Modify: `src/runtime/js/viewshell_init.js`
- Create: `src/runtime/js/viewshell_window.js`
- Create: `src/runtime/js/viewshell_webview.js`
- Create: `tests/unit/request_tracker_test.cpp`
- Create: `tests/unit/builtin_dispatcher_test.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST(RequestTracker, invalidates_pending_requests_on_bridge_reset) {
  RequestTracker tracker;
  auto id = tracker.begin_request(/* generation = */ 1);
  auto result = tracker.fail_generation(1, "bridge_reset");
  EXPECT_EQ(result.at(id).code, "bridge_reset");
}
```

```cpp
TEST(RequestTracker, expires_requests_with_bridge_timeout) {
  RequestTracker tracker;
  auto id = tracker.begin_request(/* generation = */ 1, /* timeout_ms = */ 1);
  auto result = tracker.expire_timed_out_requests_for_test();
  EXPECT_EQ(result.at(id).code, "bridge_timeout");
}
```

```cpp
TEST(RequestTracker, invalidates_requests_on_window_closed_and_bridge_unavailable) {
  RequestTracker tracker;
  auto id = tracker.begin_request(/* generation = */ 1);
  auto closed = tracker.fail_request(id, "window_closed");
  EXPECT_EQ(closed.code, "window_closed");
  auto unavailable = tracker.reject_unavailable_for_test();
  EXPECT_EQ(unavailable.code, "bridge_unavailable");
}
```

```cpp
TEST(BuiltinDispatcher, maps_set_always_on_top_opcode) {
  BuiltinDispatcher dispatcher;
  auto result = dispatcher.dispatch(BuiltinOpcode::SetAlwaysOnTop, json{{"enabled", true}}, context);
  EXPECT_TRUE(result);
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build build --target request_tracker_test builtin_dispatcher_test && ctest --test-dir build -R "RequestTracker|BuiltinDispatcher" --output-on-failure`
Expected: FAIL because request tracking and opcode dispatch do not exist.

- [ ] **Step 3: Write minimal implementation**

```cpp
enum class BuiltinOpcode {
  SetTitle,
  Maximize,
  Unmaximize,
  Minimize,
  Unminimize,
  Show,
  Hide,
  Focus,
  SetSize,
  GetSize,
  SetPosition,
  GetPosition,
  SetBorderless,
  SetAlwaysOnTop,
  BeginDrag,
  Close,
  LoadUrl,
  LoadFile,
  Reload,
  Capabilities
};
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/request_tracker.cpp src/platform/linux_x11/builtin_dispatcher.cpp)

# update `tests/CMakeLists.txt`
add_executable(request_tracker_test unit/request_tracker_test.cpp)
target_link_libraries(request_tracker_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(request_tracker_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(builtin_dispatcher_test unit/builtin_dispatcher_test.cpp)
target_link_libraries(builtin_dispatcher_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(builtin_dispatcher_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build build --target request_tracker_test builtin_dispatcher_test && ctest --test-dir build -R "RequestTracker|BuiltinDispatcher" --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt src/platform/linux_x11/request_tracker.h src/platform/linux_x11/request_tracker.cpp src/platform/linux_x11/builtin_dispatcher.h src/platform/linux_x11/builtin_dispatcher.cpp src/runtime/js/viewshell_init.js src/runtime/js/viewshell_window.js src/runtime/js/viewshell_webview.js tests/unit/request_tracker_test.cpp tests/unit/builtin_dispatcher_test.cpp
git commit -m "feat: add fast-path built-in bridge and request tracking"
```

### Task 7: Add Generic Invoke Bus And Native Event Delivery

**Files:**
- Create: `src/platform/linux_x11/invoke_bus.h`
- Create: `src/platform/linux_x11/invoke_bus.cpp`
- Create: `src/runtime/js/viewshell_invoke.js`
- Create: `src/runtime/js/viewshell_events.js`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Modify: `src/platform/linux_x11/bridge_driver.cpp`
- Modify: `src/viewshell/bridge_handle.cpp`
- Modify: `tests/unit/bridge_driver_test_hooks.h`
- Create: `tests/unit/invoke_bus_test.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST(InvokeBus, rejects_unknown_command) {
  InvokeBus bus;
  auto result = bus.dispatch("app.missing", json::object(), context);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "command_not_found");
}
```

```cpp
TEST(InvokeBus, delivers_native_events_and_off_is_idempotent) {
  InvokeBus bus;
  auto sub = bus.subscribe("native-ready", callback);
  ASSERT_TRUE(sub);
  EXPECT_TRUE(bus.emit("native-ready", json{{"ok", true}}));
  EXPECT_TRUE(sub->off());
  EXPECT_TRUE(sub->off());
}
```

```cpp
TEST(InvokeBus, bridge_emit_reports_teardown_errors) {
  BridgeHandle bridge;
  EXPECT_EQ(bridge.emit("native-ready", json::object()).error().code, "bridge_unavailable");
}
```

```cpp
TEST(InvokeBus, bridge_emit_reports_reset_and_closed_errors) {
  BridgeHandle bridge;
  EXPECT_EQ(ForceBridgeResetForTest(bridge).emit("native-ready", json::object()).error().code, "bridge_reset");
  EXPECT_EQ(ForceWindowClosedForTest(bridge).emit("native-ready", json::object()).error().code, "window_closed");
}
```

```cpp
TEST(InvokeBus, drops_subscriptions_on_reload_reset_and_window_close) {
  InvokeBus bus;
  auto sub = bus.subscribe("native-ready", callback);
  ASSERT_TRUE(sub);
  EXPECT_TRUE(ForceSubscriptionDropForTest(bus, "bridge_reset"));
  EXPECT_FALSE(bus.emit("native-ready", json{{"ok", true}}));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target invoke_bus_test && ctest --test-dir build -R InvokeBus --output-on-failure`
Expected: FAIL because generic command dispatch is missing.

- [ ] **Step 3: Write minimal implementation**

```cpp
struct NativeEvent {
  std::string name;
  json payload;
};

// Frontend event object shape:
// { name: string, payload: any }
// `tests/unit/bridge_driver_test_hooks.h` also exposes:
// - `BridgeHandle& ForceBridgeResetForTest(BridgeHandle&)`
// - `BridgeHandle& ForceWindowClosedForTest(BridgeHandle&)`
// - `bool ForceSubscriptionDropForTest(InvokeBus&, std::string_view reason)`
```

```cmake
# update `CMakeLists.txt`
target_sources(viewshell PRIVATE src/platform/linux_x11/invoke_bus.cpp)

# update `tests/CMakeLists.txt`
add_executable(invoke_bus_test unit/invoke_bus_test.cpp)
target_link_libraries(invoke_bus_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(invoke_bus_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build --target invoke_bus_test && ctest --test-dir build -R InvokeBus --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt src/platform/linux_x11/invoke_bus.h src/platform/linux_x11/invoke_bus.cpp src/runtime/js/viewshell_invoke.js src/runtime/js/viewshell_events.js src/platform/linux_x11/bridge_driver.cpp src/viewshell/bridge_handle.cpp tests/unit/bridge_driver_test_hooks.h tests/unit/invoke_bus_test.cpp
git commit -m "feat: add invoke bus and native event delivery"
```

### Task 8: Finish Example, Integration Coverage, And Size Check

**Files:**
- Create: `examples/linux_x11_smoke/main.cpp`
- Create: `examples/linux_x11_smoke/app/index.html`
- Create: `examples/linux_x11_smoke/app/main.js`
- Modify: `CMakeLists.txt`
- Modify: `tests/integration/local_app_smoke_test.cpp`
- Modify: `tests/integration/remote_untrusted_test.cpp`
- Create: `tests/integration/size_check_test.sh`

- [ ] **Step 1: Write the failing verification steps**

```bash
du -sb build/examples/linux_x11_smoke_bundle
test $? -eq 0
```

- [ ] **Step 2: Run verification to confirm it fails**

Run: `cmake --build build --target linux_x11_smoke && bash tests/integration/size_check_test.sh`
Expected: FAIL because the example bundle and size-check script do not exist yet.

- [ ] **Step 3: Write the minimal example and verification script**

```js
await Viewshell.window.setBorderless(true)
await Viewshell.window.setAlwaysOnTop(true)
await Viewshell.window.beginDrag()
const pong = await Viewshell.invoke('app.ping', { value: 1 })
```

```cpp
TEST(LocalAppSmoke, receives_native_ready_event_and_capabilities) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
  app.host_window().bridge().value().emit("native-ready", json{{"ok", true}});
  ASSERT_EQ(app.eval_async_js("new Promise(resolve => { Viewshell.on('native-ready', e => resolve(String(e.payload.ok))); })"), "true");
  ASSERT_EQ(app.eval_async_js("Viewshell.capabilities().then(c => String(c.window.borderless))"), "true");
}

TEST(RemoteUntrusted, still_has_no_bridge_in_final_bundle) {
  TestAppHarness app;
  ASSERT_TRUE(app.launch_remote("https://untrusted.example"));
  ASSERT_EQ(app.eval_js("typeof window.Viewshell"), "undefined");
}
```

```cmake
# update `CMakeLists.txt`
add_executable(linux_x11_smoke examples/linux_x11_smoke/main.cpp)
target_link_libraries(linux_x11_smoke PRIVATE viewshell)
```

```bash
# `tests/integration/size_check_test.sh` assembles:
# build/examples/linux_x11_smoke_bundle/
#   linux_x11_smoke
#   app/index.html
#   app/main.js
mkdir -p build/examples/linux_x11_smoke_bundle/app
cp build/linux_x11_smoke build/examples/linux_x11_smoke_bundle/
strip build/examples/linux_x11_smoke_bundle/linux_x11_smoke
cp examples/linux_x11_smoke/app/index.html build/examples/linux_x11_smoke_bundle/app/
cp examples/linux_x11_smoke/app/main.js build/examples/linux_x11_smoke_bundle/app/
actual_size=$(du -sb build/examples/linux_x11_smoke_bundle | cut -f1)
test "$actual_size" -lt 10485760
```

- [ ] **Step 4: Run full verification**

Run: `cmake -S . -B build && cmake --build build && xvfb-run -a ctest --test-dir build --output-on-failure && bash tests/integration/size_check_test.sh`
Expected: all tests PASS and the example output directory is `< 10485760` bytes.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt examples/linux_x11_smoke/main.cpp examples/linux_x11_smoke/app/index.html examples/linux_x11_smoke/app/main.js tests/integration/local_app_smoke_test.cpp tests/integration/remote_untrusted_test.cpp tests/integration/size_check_test.sh
git commit -m "feat: ship linux x11 smoke app and verification checks"
```
