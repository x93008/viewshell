# macOS Backend Skeleton Design

Date: 2026-04-15
Status: Approved in chat, implementation in progress

## Goal

Add a macOS backend skeleton that plugs into the existing runtime/host structure
and provides native Cocoa window management.

## Scope

- add `MacOSBackendRuntime`
- add `MacOSWindowHost`
- support basic window management
- leave webview and bridge methods as explicit `unsupported_by_backend`

## Non-goals

- WKWebView integration
- macOS bridge implementation
