# Runtime Adapter Move Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move backend runtime adapters into `src/runtime/` because they are runtime-layer concerns rather than platform buckets.

**Architecture:** Keep behavior unchanged while relocating X11 and Win32 backend runtime adapters to the runtime subsystem. Platform window hosts remain where they are for now.

**Tech Stack:** C++17, CMake, GTest, CTest

---

## Tasks

### Task 1: Move X11 and Win32 runtime adapters
- Move files into `src/runtime/`
- Update includes and CMake

### Task 2: Verification
- Run full build and tests
