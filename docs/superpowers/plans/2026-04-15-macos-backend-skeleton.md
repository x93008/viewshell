# macOS Backend Skeleton Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a macOS backend skeleton with native window management under the existing runtime/host abstractions.

**Architecture:** Introduce `MacOSBackendRuntime` under `src/runtime/` and `MacOSWindowHost` under `src/host/macos/`, using Cocoa/AppKit for the basic window operations while keeping webview and bridge as explicit stubs.

**Tech Stack:** C++17, Objective-C++, AppKit, CMake

---

## Tasks

### Task 1: Add macOS runtime and host types
- Create `src/runtime/macos_backend_runtime.h/.mm`
- Create `src/host/macos/macos_window_host.h/.mm`
- Update backend factory and CMake

### Task 2: Implement basic Cocoa window management
- Title, size, position, show/hide, focus, minimize/maximize, borderless, topmost, close

### Task 3: Verify Linux remains green and macOS config/build path works remotely
