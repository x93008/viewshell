# Promise Bridge Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `window.__viewshell.invoke(...)` return a Promise that resolves or rejects from native `invoke_result` replies.

**Architecture:** Add request IDs in the JS bootstrap, preserve them through the Linux X11 bridge path, and dispatch native invoke results back into the page so the bootstrap can settle pending promises.

**Tech Stack:** C++17, WebKitGTK, GTK3, GTest, CTest

---

## Tasks

### Task 1: Add requestId to bridge bootstrap and pending promise map
- Modify `src/platform/linux_x11/bridge_driver.cpp`
- Modify `examples/hello_viewshell/app/main.js`

### Task 2: Echo requestId through native invoke path
- Modify `src/platform/linux_x11/linux_x11_window_host.cpp`

### Task 3: Verify Promise behavior via tests and demo update
- Modify/add focused bridge tests
- Run full verification
