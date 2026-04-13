# Windows Backend Skeleton Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a Win32-based Windows backend skeleton that supports window management through the existing runtime/window host abstractions.

**Architecture:** Introduce `WindowsBackendRuntime` and `WindowsWindowHost`, wire backend selection through `BackendFactory`, and keep Windows webview/bridge methods as explicit `unsupported_by_backend` stubs until WebView2 is implemented.

**Tech Stack:** C++17, Win32 API, existing runtime/backend abstractions, CMake

---

## Tasks

### Task 1: Add Windows backend types
- Create `src/platform/windows/windows_backend_runtime.h/.cpp`
- Create `src/platform/windows/windows_window_host.h/.cpp`
- Modify `src/runtime/backend_factory.cpp`
- Modify `src/CMakeLists.txt`

### Task 2: Implement Win32 window management
- Implement title, size, position, borderless, topmost, show/hide, focus, minimize/maximize, close
- Keep webview/bridge methods as `unsupported_by_backend`

### Task 3: Verification
- Ensure Linux build/tests still pass
- Ensure Windows sources are conditionally compiled only on Windows hosts
