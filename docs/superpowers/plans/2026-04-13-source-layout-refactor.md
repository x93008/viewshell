# Source Layout Refactor Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restructure `src/` by subsystem (`window`, `webview`, `bridge`) and keep `platform/` focused on platform adapters such as `x11` and `windows`.

**Architecture:** Move implementation files into subsystem directories without changing external behavior. Rename `platform/linux_x11` to `platform/x11`, keep runtime abstractions intact, and update includes/CMake so Linux stays green and Windows remains conditionally compiled.

**Tech Stack:** C++17, CMake, GTK3/X11, WebKitGTK, Win32 skeleton, GTest, CTest

---

## Tasks

### Task 1: Rename `platform/linux_x11` to `platform/x11`
- Move the directory in git
- Update include paths and CMake references
- Build and run tests

### Task 2: Move window-specific files to `src/window/`
- Move drivers and native window types
- Update includes and CMake
- Build and run tests

### Task 3: Move webview-specific files to `src/webview/`
- Move webview driver and resource protocol
- Update includes and CMake
- Build and run tests

### Task 4: Move bridge-specific files to `src/bridge/`
- Move bridge driver, invoke bus, request tracker, builtin dispatcher, trust gate
- Update includes and CMake
- Build and run tests

### Task 5: Keep platform adapters thin and reverify
- Update platform adapters to point at subsystem files
- Run full verification
