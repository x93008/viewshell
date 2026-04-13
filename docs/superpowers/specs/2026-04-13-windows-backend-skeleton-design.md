# Windows Backend Skeleton Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Add a Windows backend skeleton that plugs into the existing backend/runtime
architecture and provides native Win32 window management for the current public
window API.

## Scope

- add `WindowsBackendRuntime`
- add `WindowsWindowHost`
- implement Win32 window management methods for title, size, position,
  borderless, always-on-top, show/hide, focus, minimize/maximize, close
- keep webview and bridge methods unimplemented and return
  `unsupported_by_backend`
- select backend by platform in `BackendFactory`

## Non-goals

- WebView2 integration
- Windows bridge implementation
- Windows demo app parity with Linux
