# Multi-Window And Floating Options Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add arbitrary multi-window support and the minimum floating-window options needed for a true two-window demo.

**Architecture:** Replace the single `RuntimeWindowState` ownership in `Application` with a collection of per-window states, let each backend runtime manage multiple active hosts, and add `show_in_taskbar` / `resizable` options carried through platform hosts.

**Tech Stack:** C++17, CMake, GTK/X11, Win32, AppKit

---

## Tasks

### Task 1: Runtime data model for multiple windows
- Modify `include/viewshell/application.h`
- Modify `src/viewshell/application.cpp`
- Modify `src/viewshell/runtime_state.h`

### Task 2: Backend runtimes track multiple hosts
- Modify `src/runtime/backend_runtime.h`
- Modify `src/runtime/x11_backend_runtime.*`
- Modify `src/runtime/win32_backend_runtime.*`
- Modify `src/runtime/macos_backend_runtime.*`

### Task 3: Add floating-window options
- Modify `include/viewshell/options.h`
- Modify platform hosts for `show_in_taskbar` and `resizable`

### Task 4: Verification
- Run Linux full build/tests
- Build Windows/macOS examples remotely
