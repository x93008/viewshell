# Windows Bridge Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a minimal WebView2-backed bridge for Windows with invoke/emit/on/off support.

**Architecture:** Reuse the existing bridge protocol shape from Linux, but route it through WebView2's web message APIs. `Win32WindowHost` owns command dispatch and subscriptions; `Win32WebviewHost` injects bootstrap script, receives messages, and posts messages back into the page.

**Tech Stack:** C++17, Win32 API, WebView2 SDK, nlohmann_json

---

## Tasks

### Task 1: Add Win32 web message bridge plumbing
- Modify `src/webview/win32_webview_host.h`
- Modify `src/webview/win32_webview_host.cpp`

### Task 2: Add Windows invoke/emit/on/off handling
- Modify `src/host/windows/win32_window_host.h`
- Modify `src/host/windows/win32_window_host.cpp`

### Task 3: Verify build and runtime behavior
- Rebuild on Windows
- Verify `hello_viewshell` bridge demo works
