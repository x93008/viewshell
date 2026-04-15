# Windows WebView Next Step Design

Date: 2026-04-14
Status: Approved in chat, implementation in progress

## Goal

Add the next useful batch of Windows WebView2 capabilities:

- devtools
- init script support
- page load callback support

## Scope

- implement `add_init_script()` on Windows WebView2
- implement `open_devtools()` / `close_devtools()` on Windows
- implement `on_page_load()` callback dispatch from WebView2 navigation events

## Non-goals

- navigation blocking policy
- Windows local resource protocol parity
- Windows bridge redesign
