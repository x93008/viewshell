# Minimal JS Native Bridge Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `window.__viewshell.invoke(...)` and `window.__viewshell.emit(...)` work end-to-end on Linux X11 WebKitGTK.

**Architecture:** Inject a bootstrap object into the page, use a WebKit script message handler for JS-to-native messages, and reuse `BridgeDriver` plus `WindowHost` to route native-to-page messages.

**Tech Stack:** C++17, WebKitGTK, GTK3, GTest, CTest

---

## Tasks

### Task 1: Add WebKit message handler plumbing
- Modify `src/platform/linux_x11/webview_driver.h`
- Modify `src/platform/linux_x11/webview_driver.cpp`
- Add callbacks for `viewshellBridge`

### Task 2: Add bridge bootstrap script and dispatch
- Modify `src/platform/linux_x11/bridge_driver.h`
- Modify `src/platform/linux_x11/bridge_driver.cpp`
- Inject `window.__viewshell.invoke/emit`

### Task 3: Wire LinuxX11WindowHost bridge messages
- Modify `src/platform/linux_x11/linux_x11_window_host.cpp`
- Route incoming raw messages into command dispatch or emit path

### Task 4: Add focused verification
- Modify/add focused bridge tests
- Run full verification
