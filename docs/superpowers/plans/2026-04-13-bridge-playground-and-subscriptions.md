# Bridge Playground And Native Subscription Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add native-backed bridge subscriptions, upgrade the hello demo into a playground, and document the bridge separately from the README.

**Architecture:** JS `on/off` posts subscribe/unsubscribe messages to native; LinuxX11 host stores subscribed event names and gates `emit(...)` on that set. The demo exposes the bridge surface clearly and keeps the console bounded and scrollable.

**Tech Stack:** C++17, WebKitGTK, GTK3, CTest, GTest

---

## Tasks

### Task 1: Native-backed on/off
- Modify `src/platform/linux_x11/bridge_driver.cpp`
- Modify `src/platform/linux_x11/linux_x11_window_host.h`
- Modify `src/platform/linux_x11/linux_x11_window_host.cpp`

### Task 2: Hello playground
- Modify `examples/hello_viewshell/app/index.html`
- Modify `examples/hello_viewshell/app/main.js`

### Task 3: Bridge docs
- Modify `README.md`
- Create `docs/bridge-api.md`
