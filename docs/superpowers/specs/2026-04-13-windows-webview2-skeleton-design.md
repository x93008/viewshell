# Windows WebView2 Skeleton Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Add a minimal runtime-integrated WebView2 skeleton for Windows so the Windows
backend can create an embedded webview, load remote URLs, and evaluate script.

## Scope

- add a Win32 WebView2 wrapper under `src/webview/`
- attach it to `Win32WindowHost`
- support `load_url()` and `evaluate_script()`
- keep local file loading, bridge, init scripts, page load callbacks,
  navigation handler, and devtools as explicit stubs for now

## Non-goals

- WebView2 bridge integration
- local resource protocol on Windows
- parity with the Linux WebKitGTK backend
