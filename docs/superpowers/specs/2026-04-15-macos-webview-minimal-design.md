# macOS WebView Minimal Design

Date: 2026-04-15
Status: Approved in chat, implementation in progress

## Goal

Add a minimal macOS WKWebView integration that supports loading remote URLs and
local files, and evaluating JavaScript.

## Scope

- embed `WKWebView` in `MacOSWindowHost`
- support `load_url`, `load_file`, `reload`, and `evaluate_script`
- resize with the host window

## Non-goals

- bridge
- page load callbacks
- init scripts
- devtools
