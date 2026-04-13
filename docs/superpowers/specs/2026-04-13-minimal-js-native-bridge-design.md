# Minimal JS Native Bridge Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Provide a minimal working JS/native bridge for Linux X11 WebKitGTK with these APIs:

- `window.__viewshell.invoke(name, args)`
- `window.__viewshell.emit(name, payload)`

This change only establishes message flow. It does not add request IDs, timeout
handling, error callbacks, or event subscriptions.

## Design

- Inject a small bootstrap script into every attached webview.
- JS -> native uses a WebKit script message handler.
- Native -> JS uses `webkit_web_view_evaluate_javascript(...)` to dispatch a
  message event into the page.
- `invoke(name, args)` forwards to the current host command path.
- `emit(name, payload)` forwards as a fire-and-forget native event path.

## Non-goals

- Promise resolution for invoke results
- Structured error propagation back to JS
- `on/off` event subscription surface
- Protocol redesign
