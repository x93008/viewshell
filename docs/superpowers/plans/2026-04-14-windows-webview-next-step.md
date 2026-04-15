# Windows WebView Next Step Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add Windows WebView2 support for devtools, init scripts, and page load callbacks.

**Architecture:** Extend `Win32WebviewHost` to track init scripts and hook WebView2 navigation events, then surface those abilities through `Win32WindowHost` while keeping the current bridge path intact.

**Tech Stack:** C++17, Win32 API, WebView2 SDK, CMake

---

## Tasks

### Task 1: Add init script support to Win32WebviewHost
- Modify `src/webview/win32_webview_host.h`
- Modify `src/webview/win32_webview_host.cpp`

### Task 2: Add devtools and page load callback support
- Modify `src/webview/win32_webview_host.h`
- Modify `src/webview/win32_webview_host.cpp`
- Modify `src/host/windows/win32_window_host.cpp`

### Task 3: Verify Linux and Windows build paths
- Run Linux full build/tests
- Run Windows build of example targets
