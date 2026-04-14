# Windows Bridge Design

Date: 2026-04-14
Status: Approved in chat, implementation in progress

## Goal

Add a minimal Windows bridge implementation using WebView2 web messages so the
Windows backend supports `invoke`, `emit`, and `on/off` subscription semantics.

## Scope

- JS bootstrap injection on Windows
- Promise-based `invoke`
- `emit`
- native-backed `on/off`
- routing via `Win32WindowHost` and `InvokeBus`

## Non-goals

- devtools
- page load callback
- navigation handler
