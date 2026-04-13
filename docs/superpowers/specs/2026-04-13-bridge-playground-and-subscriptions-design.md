# Bridge Playground And Native Subscription Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Implement a real native-backed `window.__viewshell.on/off` subscription path,
upgrade `hello_viewshell` into a bridge playground, and document the bridge in a
dedicated document.

## Scope

- JS `on/off` sends subscribe/unsubscribe messages to native
- native tracks subscriptions and only emits events to subscribed pages
- `hello_viewshell` demonstrates resolve, reject, subscribe, unsubscribe, and
  keeps a bounded scrollable message log
- add standalone bridge documentation
