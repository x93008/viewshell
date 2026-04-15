# macOS Minimal Bridge Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a minimal macOS bridge for `invoke/emit/on/off` and circle-window drag/close behavior.

**Architecture:** Use `WKUserContentController` and `WKScriptMessageHandler` to receive messages from the page, reuse `InvokeBus` and subscription storage in `MacOSWindowHost`, and inject a small bootstrap script that mirrors the existing Windows/Linux JS surface.

**Tech Stack:** C++17, Objective-C++, AppKit, WebKit, nlohmann_json

---

## Tasks

### Task 1: Add WKWebView user-content bridge plumbing
- Modify `src/host/macos/macos_window_host.h`
- Modify `src/host/macos/macos_window_host.mm`

### Task 2: Add transparent borderless handling and drag/close hooks
- Modify `src/host/macos/macos_window_host.mm`
- Modify `examples/circle_window/app/main.js`

### Task 3: Verify on Linux and remote macOS
