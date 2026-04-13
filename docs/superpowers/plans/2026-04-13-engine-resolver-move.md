# Engine Resolver Move Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move the current X11/Linux engine resolver into `src/webview/engine_resolver.*` and align naming with its real responsibility.

**Architecture:** Keep behavior unchanged while renaming the implementation to `EngineResolver` and relocating it into the webview subsystem. Platform runtime continues to call the resolver through the new location.

**Tech Stack:** C++17, CMake, GTest, CTest

---

## Tasks

### Task 1: Move and rename resolver
- Move `src/platform/x11/kernel_resolver.*` to `src/webview/engine_resolver.*`
- Rename `KernelResolver` to `EngineResolver`

### Task 2: Update call sites and tests
- Update runtime, tests, and support headers
- Update CMake source list

### Task 3: Verify
- Run full build and tests
