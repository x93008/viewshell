# macOS WebView Minimal Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a minimal embedded WKWebView to the macOS backend so examples can load local frontend files and evaluate JavaScript.

**Architecture:** Extend `MacOSWindowHost` to own a `WKWebView` attached to the NSWindow content view. Implement `load_url`, `load_file`, `reload`, and `evaluate_script`, while keeping advanced features stubbed.

**Tech Stack:** C++17, Objective-C++, AppKit, WebKit, CMake

---

## Tasks

### Task 1: Add WKWebView ownership to MacOSWindowHost
- Modify `src/host/macos/macos_window_host.h`
- Modify `src/host/macos/macos_window_host.mm`

### Task 2: Implement minimal webview operations
- `load_url`
- `load_file`
- `reload`
- `evaluate_script`

### Task 3: Verify Linux and remote macOS builds
