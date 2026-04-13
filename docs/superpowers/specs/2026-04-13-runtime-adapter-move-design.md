# Runtime Adapter Move Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Move platform-specific backend runtime adapters out of `src/platform/` into the
`src/runtime/` subsystem, while leaving window hosts in place for now.

## Scope

- move `src/platform/x11/x11_backend_runtime.*` to `src/runtime/x11_backend_runtime.*`
- move `src/platform/windows/win32_backend_runtime.*` to `src/runtime/win32_backend_runtime.*`
- update includes, CMake, and backend factory

## Non-goals

- moving window hosts in this change
- deleting `src/platform/` entirely in this change
