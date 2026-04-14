# Windows WebView2 Skeleton Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Integrate a minimal WebView2 host into the Windows backend so `load_url()` and `evaluate_script()` are functional on Windows.

**Architecture:** Add `win32_webview_host.*` under `src/webview/`, let `Win32WindowHost` own it, and wire the simplest attach/load/evaluate path. Leave advanced webview and bridge features as explicit stubs.

**Tech Stack:** C++17, Win32 API, WebView2 SDK, CMake

---

## Tasks

### Task 1: Add Win32 WebView2 wrapper
- Create `src/webview/win32_webview_host.h`
- Create `src/webview/win32_webview_host.cpp`

### Task 2: Wire Win32WindowHost to WebView2
- Modify `src/host/windows/win32_window_host.h`
- Modify `src/host/windows/win32_window_host.cpp`
- Modify `src/CMakeLists.txt`

### Task 3: Verify Linux remains green
- Build and run full Linux test suite
