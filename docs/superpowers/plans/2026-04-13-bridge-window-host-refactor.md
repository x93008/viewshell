# Bridge Window Host Refactor Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move bridge registration and emission behind `WindowHost`, and make `LinuxX11WindowHost` own bridge composition.

**Architecture:** Extend `WindowHost` with bridge methods, route `BridgeHandle` through that abstraction, and compose `BridgeDriver` plus `InvokeBus` inside `LinuxX11WindowHost` so bridge ownership matches the new runtime/backend structure.

**Tech Stack:** C++17, GTK3/X11, WebKitGTK, GTest, CTest

---

## File Structure

- Modify: `src/runtime/window_host.h`
- Modify: `include/viewshell/bridge_handle.h`
- Modify: `src/viewshell/bridge_handle.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Modify: `src/platform/linux_x11/linux_x11_window_host.h`
- Modify: `src/platform/linux_x11/linux_x11_window_host.cpp`
- Modify: `src/platform/linux_x11/bridge_driver.h`
- Modify: `src/platform/linux_x11/bridge_driver.cpp`
- Modify: `src/platform/linux_x11/invoke_bus.h`
- Modify: `src/platform/linux_x11/invoke_bus.cpp`
- Modify: `tests/unit/application_lifecycle_test.cpp`
- Modify: `tests/unit/window_lifecycle_test.cpp`

## Chunk 1: Route BridgeHandle Through WindowHost

### Task 1: Add bridge methods to `WindowHost`

**Files:**
- Modify: `src/runtime/window_host.h`
- Modify: `tests/unit/window_lifecycle_test.cpp`

- [ ] Add pure virtual `register_command(...)` and `emit(...)` to `WindowHost`
- [ ] Extend the recording test host in `tests/unit/window_lifecycle_test.cpp`
- [ ] Add a failing/passing test proving `BridgeHandle` can delegate through a host-backed runtime state
- [ ] Build and run `window_lifecycle_test`

### Task 2: Refactor `BridgeHandle`

**Files:**
- Modify: `src/viewshell/bridge_handle.cpp`
- Modify: `src/viewshell/runtime_state.h`
- Modify: `tests/unit/application_lifecycle_test.cpp`

- [ ] Remove direct command registry mutation from `BridgeHandle`
- [ ] Delegate `register_command(...)` and `emit(...)` to `state_->window_host`
- [ ] Preserve `window_closed`, `command_already_registered`, and `bridge_unavailable` behavior
- [ ] Build and run focused application/window lifecycle tests

## Chunk 2: Move Bridge Ownership Into LinuxX11WindowHost

### Task 3: Add bridge composition to `LinuxX11WindowHost`

**Files:**
- Modify: `src/platform/linux_x11/linux_x11_window_host.h`
- Modify: `src/platform/linux_x11/linux_x11_window_host.cpp`
- Modify: `src/platform/linux_x11/bridge_driver.h`
- Modify: `src/platform/linux_x11/bridge_driver.cpp`
- Modify: `src/platform/linux_x11/invoke_bus.h`
- Modify: `src/platform/linux_x11/invoke_bus.cpp`

- [ ] Make `LinuxX11WindowHost` own `BridgeDriver` and `InvokeBus`
- [ ] Register bridge commands in `InvokeBus`
- [ ] Keep current bridge activation behavior stable
- [ ] Remove no-longer-needed bridge storage from `RuntimeWindowState`
- [ ] Run bridge-related unit tests

## Chunk 3: Verification

### Task 4: Full verification

**Files:**
- Modify: any files touched above as needed

- [ ] Run `cmake --build --preset conan-debug`
- [ ] Run `ctest --preset conan-debug --output-on-failure`
- [ ] Manually verify examples still launch with timeout-based smoke check
- [ ] Commit
