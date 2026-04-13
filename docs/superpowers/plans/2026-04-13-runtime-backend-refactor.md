# Runtime Backend Refactor Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Introduce an explicit backend runtime and window host abstraction so the public runtime is no longer directly coupled to Linux/X11 concrete drivers.

**Architecture:** Add a narrow `BackendRuntime` abstraction for app-level backend ownership, a `WindowHost` abstraction for one concrete window instance, and Linux/X11 implementations of both. `Application` delegates backend work to `BackendRuntime`, and `WindowHandle` delegates window/webview work to `WindowHost`, while `KernelResolver` becomes part of actual backend creation.

**Tech Stack:** C++17, CMake, GTK3/X11, WebKitGTK, CTest, GTest

---

## File Structure

### New runtime abstraction files

- Create: `src/runtime/backend_runtime.h`
- Create: `src/runtime/window_host.h`
- Create: `src/runtime/backend_factory.h`
- Create: `src/runtime/backend_factory.cpp`

Responsibilities:

- `backend_runtime.h`: backend runtime interface for app-level backend behavior
- `window_host.h`: window instance interface used by `WindowHandle`
- `backend_factory.h/.cpp`: backend selection and creation entry point

### New Linux X11 backend composition files

- Create: `src/platform/linux_x11/linux_x11_backend_runtime.h`
- Create: `src/platform/linux_x11/linux_x11_backend_runtime.cpp`
- Create: `src/platform/linux_x11/linux_x11_window_host.h`
- Create: `src/platform/linux_x11/linux_x11_window_host.cpp`

Responsibilities:

- `linux_x11_backend_runtime.*`: owns Linux/X11 backend runtime wiring, resolver hookup, and run/post delegation
- `linux_x11_window_host.*`: owns `WindowDriver` and `WebviewDriver` composition for one concrete window

### Existing files to modify

- Modify: `include/viewshell/application.h`
- Modify: `src/viewshell/application.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `src/CMakeLists.txt`
- Modify: `tests/unit/application_lifecycle_test.cpp`
- Modify: `tests/unit/window_lifecycle_test.cpp`
- Modify: `tests/unit/runtime_test_hooks.cpp`
- Modify: `tests/unit/runtime_test_hooks.h`

Responsibilities:

- `application.*`: replace concrete driver ownership with backend runtime ownership
- `runtime_state.h`: replace concrete driver pointers with `WindowHost`
- `window_handle.cpp`: delegate to `WindowHost`
- `src/CMakeLists.txt`: compile new runtime/backend files
- tests: update any test hooks that reach into concrete driver state

## Chunk 1: Add Runtime And Window Host Abstractions

### Task 1: Introduce `WindowHost` interface

**Files:**
- Create: `src/runtime/window_host.h`
- Modify: `src/viewshell/runtime_state.h`
- Test: `tests/unit/window_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing test**

Add a test that only depends on the presence of a window host in runtime state:

```cpp
TEST(WindowHandleLifecycle, reports_unsupported_backend_when_no_window_host_present) {
  auto state = std::make_shared<viewshell::RuntimeWindowState>();
  state->has_window = true;
  viewshell::WindowHandle handle(state);

  auto result = handle.load_url("https://example.com");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "unsupported_by_backend");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target window_lifecycle_test && ctest --preset conan-debug -R WindowHandleLifecycle --output-on-failure`
Expected: FAIL because `RuntimeWindowState` still exposes concrete driver pointers only.

- [ ] **Step 3: Write minimal abstraction**

Create `src/runtime/window_host.h` with methods matching the current `WindowHandle`
delegation needs:

```cpp
class WindowHost {
public:
  virtual ~WindowHost() = default;
  virtual Result<void> set_title(std::string_view title) = 0;
  virtual Result<void> maximize() = 0;
  virtual Result<void> unmaximize() = 0;
  virtual Result<void> minimize() = 0;
  virtual Result<void> unminimize() = 0;
  virtual Result<void> show() = 0;
  virtual Result<void> hide() = 0;
  virtual Result<void> focus() = 0;
  virtual Result<void> set_size(Size size) = 0;
  virtual Result<Size> get_size() const = 0;
  virtual Result<void> set_position(Position pos) = 0;
  virtual Result<Position> get_position() const = 0;
  virtual Result<void> set_borderless(bool enabled) = 0;
  virtual Result<void> set_always_on_top(bool enabled) = 0;
  virtual Result<void> close() = 0;
  virtual Result<void> load_url(std::string_view url) = 0;
  virtual Result<void> load_file(std::string_view entry_file) = 0;
  virtual Result<void> reload() = 0;
  virtual Result<void> evaluate_script(std::string_view script) = 0;
  virtual Result<void> add_init_script(std::string_view script) = 0;
  virtual Result<void> open_devtools() = 0;
  virtual Result<void> close_devtools() = 0;
  virtual Result<void> on_page_load(PageLoadHandler handler) = 0;
  virtual Result<void> set_navigation_handler(NavigationHandler handler) = 0;
  virtual Result<Capabilities> capabilities() const = 0;
};
```

Update `RuntimeWindowState` to hold:

```cpp
std::shared_ptr<WindowHost> window_host;
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target window_lifecycle_test && ctest --preset conan-debug -R WindowHandleLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/runtime/window_host.h src/viewshell/runtime_state.h tests/unit/window_lifecycle_test.cpp
git commit -m "refactor: add window host abstraction to runtime state"
```

### Task 2: Introduce `BackendRuntime` and `BackendFactory`

**Files:**
- Create: `src/runtime/backend_runtime.h`
- Create: `src/runtime/backend_factory.h`
- Create: `src/runtime/backend_factory.cpp`
- Modify: `include/viewshell/application.h`
- Test: `tests/unit/application_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing test**

Add a test that asserts `Application::run()` can fail cleanly when no backend runtime exists yet.

```cpp
TEST(ApplicationLifecycle, run_requires_backend_runtime_after_window_creation_path) {
  auto app = viewshell::Application::create(viewshell::AppOptions{});
  ASSERT_TRUE(app);
  auto result = app->run();
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_state");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target application_lifecycle_test && ctest --preset conan-debug -R ApplicationLifecycle --output-on-failure`
Expected: FAIL until backend runtime ownership is represented in the public runtime.

- [ ] **Step 3: Write minimal abstraction**

Create `BackendRuntime`:

```cpp
class BackendRuntime {
public:
  virtual ~BackendRuntime() = default;
  virtual Result<std::shared_ptr<WindowHost>> create_window(
      std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state,
      const NormalizedAppOptions& app_options,
      const WindowOptions& window_options) = 0;
  virtual Result<void> post(std::shared_ptr<RuntimeAppState> app_state,
      std::function<void()> task) = 0;
  virtual Result<int> run(std::shared_ptr<RuntimeAppState> app_state,
      std::shared_ptr<RuntimeWindowState> window_state) = 0;
};
```

Create `BackendFactory::create(...)` returning `std::unique_ptr<BackendRuntime>`.

Update `Application` to own:

```cpp
std::unique_ptr<BackendRuntime> backend_runtime_;
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target application_lifecycle_test && ctest --preset conan-debug -R ApplicationLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/runtime/backend_runtime.h src/runtime/backend_factory.h src/runtime/backend_factory.cpp include/viewshell/application.h tests/unit/application_lifecycle_test.cpp
git commit -m "refactor: add backend runtime abstraction and factory"
```

## Chunk 2: Add Linux X11 Concrete Backend Composition

### Task 3: Add `LinuxX11WindowHost`

**Files:**
- Create: `src/platform/linux_x11/linux_x11_window_host.h`
- Create: `src/platform/linux_x11/linux_x11_window_host.cpp`
- Modify: `src/CMakeLists.txt`
- Test: `tests/unit/window_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing test**

Add a test that a host-backed `WindowHandle` forwards to a host implementation.

```cpp
TEST(WindowHandleLifecycle, delegates_window_ops_to_window_host) {
  // Use a tiny fake host that records the title passed in.
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target window_lifecycle_test && ctest --preset conan-debug -R WindowHandleLifecycle --output-on-failure`
Expected: FAIL because `WindowHandle` still dispatches to concrete drivers.

- [ ] **Step 3: Write minimal Linux host**

Implement `LinuxX11WindowHost` that owns:

```cpp
std::unique_ptr<WindowDriver> window_driver_;
std::unique_ptr<WebviewDriver> webview_driver_;
```

It should:

- create the native window through `WindowDriver`
- attach the webview through `WebviewDriver`
- load local assets when `asset_root` is set
- wire close callback to runtime state shutdown
- implement all `WindowHost` virtual methods by forwarding

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target window_lifecycle_test && ctest --preset conan-debug -R WindowHandleLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/platform/linux_x11/linux_x11_window_host.h src/platform/linux_x11/linux_x11_window_host.cpp src/CMakeLists.txt tests/unit/window_lifecycle_test.cpp
git commit -m "refactor: add linux x11 window host"
```

### Task 4: Add `LinuxX11BackendRuntime`

**Files:**
- Create: `src/platform/linux_x11/linux_x11_backend_runtime.h`
- Create: `src/platform/linux_x11/linux_x11_backend_runtime.cpp`
- Modify: `src/runtime/backend_factory.cpp`
- Test: `tests/unit/application_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing test**

Add a test that backend factory creation returns a runtime object.

```cpp
TEST(ApplicationLifecycle, backend_factory_creates_runtime) {
  auto runtime = viewshell::BackendFactory::create();
  ASSERT_NE(runtime, nullptr);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target application_lifecycle_test && ctest --preset conan-debug -R ApplicationLifecycle --output-on-failure`
Expected: FAIL because the factory does not yet create a Linux backend runtime.

- [ ] **Step 3: Write minimal Linux backend runtime**

Implement `LinuxX11BackendRuntime`:

- `create_window(...)` creates `LinuxX11WindowHost`
- `post(...)` preserves current posted task queue behavior
- `run(...)` delegates to the created host/backend event loop

Update `BackendFactory::create()` to return `LinuxX11BackendRuntime`.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target application_lifecycle_test && ctest --preset conan-debug -R ApplicationLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/platform/linux_x11/linux_x11_backend_runtime.h src/platform/linux_x11/linux_x11_backend_runtime.cpp src/runtime/backend_factory.cpp tests/unit/application_lifecycle_test.cpp
git commit -m "refactor: add linux x11 backend runtime"
```

## Chunk 3: Move Public Runtime To New Abstractions

### Task 5: Refactor `Application` to use `BackendRuntime`

**Files:**
- Modify: `src/viewshell/application.cpp`
- Modify: `include/viewshell/application.h`
- Modify: `tests/unit/runtime_test_hooks.cpp`
- Modify: `tests/unit/runtime_test_hooks.h`
- Test: `tests/unit/application_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing test**

Add or update a test that asserts `create_window()` goes through runtime creation without concrete driver ownership in `Application`.

```cpp
TEST(ApplicationLifecycle, create_window_stores_window_host_not_concrete_drivers) {
  // Use test hooks to verify state after create_window.
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target application_lifecycle_test && ctest --preset conan-debug -R ApplicationLifecycle --output-on-failure`
Expected: FAIL because `Application` still directly manages concrete drivers.

- [ ] **Step 3: Refactor `Application` minimally**

Change `Application` to:

- lazily create `backend_runtime_` through `BackendFactory`
- call `backend_runtime_->create_window(...)`
- store returned `WindowHost` in `RuntimeWindowState`
- call `backend_runtime_->run(...)`
- call `backend_runtime_->post(...)`

Remove:

- `std::unique_ptr<WindowDriver> window_driver_`
- `std::unique_ptr<WebviewDriver> webview_driver_`

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target application_lifecycle_test && ctest --preset conan-debug -R ApplicationLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add include/viewshell/application.h src/viewshell/application.cpp tests/unit/runtime_test_hooks.h tests/unit/runtime_test_hooks.cpp tests/unit/application_lifecycle_test.cpp
git commit -m "refactor: route application lifecycle through backend runtime"
```

### Task 6: Refactor `WindowHandle` to use `WindowHost`

**Files:**
- Modify: `src/viewshell/window_handle.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Test: `tests/unit/window_lifecycle_test.cpp`

- [ ] **Step 1: Write the failing test**

Add a host-based forwarding test for both window and webview ops.

```cpp
TEST(WindowHandleLifecycle, delegates_webview_ops_to_window_host) {
  // Fake host records last load_url request.
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target window_lifecycle_test && ctest --preset conan-debug -R WindowHandleLifecycle --output-on-failure`
Expected: FAIL because `WindowHandle` still talks to concrete driver pointers.

- [ ] **Step 3: Refactor `WindowHandle` minimally**

Replace all forwarding like:

```cpp
state_->window_driver->set_title(...)
state_->webview_driver->load_url(...)
```

with:

```cpp
state_->window_host->set_title(...)
state_->window_host->load_url(...)
```

Remove concrete driver pointers from `RuntimeWindowState`.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target window_lifecycle_test && ctest --preset conan-debug -R WindowHandleLifecycle --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/viewshell/window_handle.cpp src/viewshell/runtime_state.h tests/unit/window_lifecycle_test.cpp
git commit -m "refactor: route window handle operations through window host"
```

## Chunk 4: Connect Resolver And Re-verify End-to-End

### Task 7: Connect `KernelResolver` into backend creation path

**Files:**
- Modify: `src/platform/linux_x11/linux_x11_backend_runtime.cpp`
- Test: `tests/unit/kernel_resolver_test.cpp`
- Test: `tests/integration/local_app_smoke_test.cpp`

- [ ] **Step 1: Write the failing test**

Add a test that backend runtime uses resolved capabilities when creating a window host.

```cpp
TEST(KernelResolver, backend_runtime_uses_resolved_capabilities) {
  // Verify resolved capabilities are surfaced into runtime/window state.
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build --preset conan-debug --target kernel_resolver_test local_app_smoke_test && ctest --preset conan-debug -R "KernelResolver|LocalAppSmoke" --output-on-failure`
Expected: FAIL because runtime creation does not yet consume resolver output.

- [ ] **Step 3: Write minimal integration**

In `LinuxX11BackendRuntime::create_window(...)`:

- call `KernelResolver::resolve(...)`
- use returned capabilities to initialize runtime/window state
- pass any engine-specific information needed by the Linux host

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build --preset conan-debug --target kernel_resolver_test local_app_smoke_test && ctest --preset conan-debug -R "KernelResolver|LocalAppSmoke" --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/platform/linux_x11/linux_x11_backend_runtime.cpp tests/unit/kernel_resolver_test.cpp tests/integration/local_app_smoke_test.cpp
git commit -m "refactor: connect kernel resolver to backend runtime creation"
```

### Task 8: Full verification and cleanup

**Files:**
- Modify: any touched files from earlier tasks as needed

- [ ] **Step 1: Run full build**

Run: `cmake --preset conan-debug --fresh && cmake --build --preset conan-debug`
Expected: PASS.

- [ ] **Step 2: Run full test suite**

Run: `ctest --preset conan-debug --output-on-failure`
Expected: PASS all tests.

- [ ] **Step 3: Run install verification**

Run: `cmake --install build/Debug`
Expected: install tree still contains library, headers, and examples.

- [ ] **Step 4: Manually verify demos still launch**

Run:

```bash
timeout 3 ./build/Debug/examples/hello_viewshell/hello_viewshell
timeout 3 ./build/Debug/examples/circle_window/circle_window
```

Expected: timeout exit indicates GTK main loop is running rather than immediate process exit.

- [ ] **Step 5: Commit**

```bash
git add .
git commit -m "refactor: complete runtime backend abstraction"
```
