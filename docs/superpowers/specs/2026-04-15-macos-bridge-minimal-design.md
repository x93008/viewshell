# macOS Minimal Bridge Design

Date: 2026-04-15
Status: Approved in chat, implementation in progress

## Goal

Add a minimal macOS bridge so `hello_viewshell` and `circle_window` can use the
same basic JS/native message flow as the other platforms.

## Scope

- inject `window.__viewshell`
- support `invoke`, `emit`, `on`, `off`
- support drag/close helpers for `circle_window`
- support transparent background for borderless windows

## Non-goals

- full macOS parity for page load, init scripts, or devtools in this step
