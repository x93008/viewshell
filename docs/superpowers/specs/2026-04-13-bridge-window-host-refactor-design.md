# Viewshell Bridge Window Host Refactor Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Summary

This refactor moves bridge ownership and dispatch out of `RuntimeWindowState` and
into the active `WindowHost`, so bridge behavior follows the same architecture as
window and webview behavior introduced by the runtime/backend refactor.

After this change, `BridgeHandle` talks to `WindowHost`, and Linux/X11 bridge
implementation details stay inside `LinuxX11WindowHost`.

## Problem

The runtime/backend refactor moved window and webview operations behind
`WindowHost`, but bridge behavior still lives in an older shape:

- `BridgeHandle` directly mutates `RuntimeWindowState::command_registry`
- `RuntimeWindowState` still stores bridge-facing state
- `BridgeDriver` and `InvokeBus` are not owned by the host that already owns the
  window and webview

That leaves bridge as the last major subsystem not aligned with the new runtime
architecture.

## Goals

1. Move bridge registration and emission behind `WindowHost`.
2. Make `LinuxX11WindowHost` the owner of `BridgeDriver` and `InvokeBus`.
3. Remove bridge-specific storage from `RuntimeWindowState` where it is no longer
   needed.
4. Preserve current public `BridgeHandle` API and error semantics.

## Non-goals

- Completing full JS/native invoke IPC semantics beyond current behavior.
- Designing a cross-platform `BridgeDriver` hierarchy.
- Expanding the public bridge API.

## Chosen approach

Add bridge methods to `WindowHost`, route `BridgeHandle` through those methods,
and compose `BridgeDriver` plus `InvokeBus` inside `LinuxX11WindowHost`.

## Architecture

### Public layer

`BridgeHandle` remains public and stable.

Responsibilities after refactor:

- Validate `window_closed`
- Delegate `register_command(...)` to `WindowHost`
- Delegate `emit(...)` to `WindowHost`

### Window host layer

Extend `WindowHost` with bridge operations:

- `register_command(std::string, CommandHandler)`
- `emit(std::string, const Json&)`

This keeps all per-window subsystems under one abstraction.

### Linux/X11 implementation

`LinuxX11WindowHost` owns:

- `WindowDriver`
- `WebviewDriver`
- `BridgeDriver`
- `InvokeBus`

`LinuxX11WindowHost` is responsible for:

- registering commands in the invoke bus
- routing `emit(...)` through the active bridge path
- keeping bridge ownership tied to window lifecycle

## State ownership changes

### `RuntimeWindowState`

Before:

- stores `command_registry`

After:

- does not own command registration storage
- keeps only public shared state still needed across handles

Bridge command/event ownership moves into `LinuxX11WindowHost`.

## Error behavior

External behavior remains stable:

- duplicate command registration still returns `command_already_registered`
- emit before bridge activation still returns `bridge_unavailable`
- closed window still returns `window_closed`

## Testing

Required coverage:

- `BridgeHandle` registers commands through host delegation
- duplicate registration remains rejected
- emit still reports `bridge_unavailable` when bridge is inactive
- window lifecycle tests still pass after command registry moves out of runtime state

## Migration sequence

1. Add bridge methods to `WindowHost`.
2. Update `BridgeHandle` to delegate through `WindowHost`.
3. Add `BridgeDriver` and `InvokeBus` ownership to `LinuxX11WindowHost`.
4. Move command registration storage into the host.
5. Remove bridge-specific runtime state that is no longer needed.
6. Verify bridge, lifecycle, and integration tests.
