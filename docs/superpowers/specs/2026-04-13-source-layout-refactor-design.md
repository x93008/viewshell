# Source Layout Refactor Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Refactor the `src/` layout so that functionality is grouped by subsystem first
and platform second, instead of keeping most implementation files flattened under
`src/platform/<platform>`.

## Target layout

```text
src/
  bridge/
  platform/
    windows/
    x11/
  runtime/
  viewshell/
  webview/
  window/
```

## Design

- `src/window/`
  - window management abstractions and platform-facing window drivers
- `src/webview/`
  - webview drivers, resource protocol, injection helpers
- `src/bridge/`
  - invoke bus, bridge driver, request tracking, trust gate, builtin dispatcher
- `src/platform/`
  - only platform-specific runtime/window-host adapters and platform-only types
  - `linux_x11` is renamed to `x11`

## Scope

- move files without changing public API
- update includes and CMake layout
- keep Linux and current tests passing
- keep Windows skeleton compiling conditionally

## Non-goals

- changing behavior
- introducing new runtime abstractions
- implementing new features during the layout move
