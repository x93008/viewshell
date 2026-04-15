# Navigation Handler Design

Date: 2026-04-15
Status: Approved in chat, implementation in progress

## Goal

Make the existing `set_navigation_handler()` API actually enforce top-level
navigation policy on Linux and Windows.

## Scope

- Linux: connect WebKitGTK navigation decision signal to the stored handler
- Windows: connect WebView2 `NavigationStarting` to the stored handler
- only cover top-level navigations

## Non-goals

- subframe navigation policy
- popup interception
- broader policy system redesign
