# Navigation Handler Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Enforce the existing navigation handler API on Linux and Windows for top-level navigations.

**Architecture:** Reuse the stored `NavigationHandler` in each platform webview implementation. On Linux, connect WebKitGTK navigation policy callbacks; on Windows, connect WebView2 `NavigationStarting` and cancel disallowed navigations.

**Tech Stack:** C++17, WebKitGTK, WebView2, CMake

---

## Tasks

### Task 1: Linux navigation handler enforcement
- Modify `src/webview/x11_webview_driver.h`
- Modify `src/webview/x11_webview_driver.cpp`

### Task 2: Windows navigation handler enforcement
- Modify `src/webview/win32_webview_host.h`
- Modify `src/webview/win32_webview_host.cpp`
- Modify `src/host/windows/win32_window_host.cpp`

### Task 3: Verification
- Run full Linux build/tests
- Validate Windows build path remotely
