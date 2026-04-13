# Promise Bridge Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Make `window.__viewshell.invoke(name, args)` return a real JavaScript `Promise`
that resolves or rejects when native sends back an `invoke_result` message.

## Scope

- JS allocates request IDs
- native echoes request IDs in invoke results
- JS tracks pending requests and resolves/rejects the corresponding promises
- `emit(...)` remains fire-and-forget

## Non-goals

- Timeout timers in JS
- Native-side timeout expiry wiring into JS in this change
- Subscription/event APIs
