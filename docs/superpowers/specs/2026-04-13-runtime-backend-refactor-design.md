# Viewshell Runtime Backend Refactor Design

Date: 2026-04-13
Status: Approved in chat, pending implementation planning

## Summary

This refactor introduces an explicit runtime/backend composition layer between the
public `Application` and `WindowHandle` APIs and the Linux X11 concrete driver
implementations.

The goal is to keep the current Linux/X11 behavior working while removing direct
knowledge of `WindowDriver` and `WebviewDriver` from `Application` and
`WindowHandle`, so future Windows and macOS backends can plug into the same
public API without repeating the current coupling.

## Problem

The current code exposes a stable public API, but the runtime wiring is still
platform-specific:

- `Application::create_window(...)` directly constructs `WindowDriver` and
  `WebviewDriver`
- `Application::run()` directly calls the Linux/X11 event loop through
  `WindowDriver`
- `WindowHandle` dispatches directly to Linux/X11 concrete driver pointers held
  in `RuntimeWindowState`
- `KernelResolver` exists but is not part of the runtime creation path

This means the public runtime has an API abstraction, but not a backend
abstraction.

## Goals

1. Introduce a backend runtime abstraction that owns platform-specific runtime
   behavior.
2. Introduce a window host abstraction that owns one concrete window instance
   and its attached subsystems.
3. Move Linux/X11 driver creation and wiring out of `Application`.
4. Move concrete driver dispatch out of `WindowHandle`.
5. Connect `KernelResolver` into actual backend startup.
6. Preserve the current public API surface and behavior.

## Non-goals

- Adding a second real backend in this change.
- Replacing every concrete driver with a pure virtual interface hierarchy.
- Changing public API method names or external error codes.
- Reworking IPC semantics beyond what is needed to relocate ownership.

## Approaches Considered

### Approach A: Add a backend runtime and window host layer

Add two narrow abstractions:

- `BackendRuntime` for app-level backend ownership
- `WindowHost` for one concrete window instance

Keep concrete Linux/X11 drivers as implementation details of the Linux backend.

Pros:

- Minimal abstraction needed to decouple public runtime from platform code
- Preserves working Linux/X11 code paths
- Creates a clean insertion point for future backends
- Keeps interface design grounded in an actually working backend

Cons:

- Concrete drivers are still platform-specific classes, not generic interfaces
- Some runtime state still needs careful ownership cleanup during transition

### Approach B: Split every subsystem into generic driver interfaces

Create `IWindowDriver`, `IWebviewDriver`, `IBridgeDriver`, and related interfaces
immediately.

Pros:

- Maximum genericity
- Forces explicit boundaries for every subsystem

Cons:

- Larger refactor surface
- Harder to validate with only one backend implementation
- Higher risk of speculative interfaces that do not fit future backends well

### Approach C: Full backend registry/plugin architecture now

Add descriptors, registration, and backend selection infrastructure before the
runtime is internally clean.

Pros:

- Most future-ready on paper

Cons:

- Too heavy for current scope
- Solves future extensibility before present code ownership is corrected
- Adds indirection without yet having multiple backends to justify it

## Chosen Approach

Approach A.

It creates the missing architecture layer with the least speculative design.
It solves the real current problem: public runtime types are coupled to Linux/X11
implementation classes.

## Architecture

### Public runtime

`Application` remains the public application owner.

Responsibilities after refactor:

- Normalize and retain `AppOptions`
- Enforce public lifecycle rules such as single-window support
- Delegate backend work to `BackendRuntime`

`WindowHandle` remains the public window handle.

Responsibilities after refactor:

- Validate lifecycle state (`window_closed`, etc.)
- Delegate window/webview operations to `WindowHost`

### Backend runtime layer

Create `src/runtime/backend_runtime.h`.

`BackendRuntime` abstracts platform runtime ownership and app-level backend work.

Responsibilities:

- Create a concrete `WindowHost`
- Own the backend main loop implementation
- Own backend-level posting and shutdown behavior
- Expose backend-resolved capabilities to the created window host

Expected shape:

- `create_window(...) -> Result<std::shared_ptr<WindowHost>>`
- `run(...) -> Result<int>`
- `post(...) -> Result<void>`

Create `src/runtime/backend_factory.h` and `src/runtime/backend_factory.cpp`.

`BackendFactory` selects the active backend runtime.

Current behavior:

- Linux/X11 only
- Returns `LinuxX11BackendRuntime`

Future behavior:

- Select Windows/macOS runtime implementations without changing public API code

### Window host layer

Create `src/runtime/window_host.h`.

`WindowHost` abstracts one live native window instance and its attached webview
and bridge machinery.

Responsibilities:

- Window operations: title, size, position, borderless, focus, close
- Webview operations: load URL/file, reload, eval, init scripts, devtools
- Lifecycle callbacks into runtime state
- Capability reporting

`WindowHandle` talks only to `WindowHost` after refactor.

### Linux X11 implementation

Create:

- `src/platform/linux_x11/linux_x11_backend_runtime.h`
- `src/platform/linux_x11/linux_x11_backend_runtime.cpp`
- `src/platform/linux_x11/linux_x11_window_host.h`
- `src/platform/linux_x11/linux_x11_window_host.cpp`

`LinuxX11BackendRuntime` responsibilities:

- Call `KernelResolver`
- Create one `LinuxX11WindowHost`
- Own Linux/X11 event loop behavior

`LinuxX11WindowHost` responsibilities:

- Own `WindowDriver`
- Own `WebviewDriver`
- Later own `BridgeDriver`
- Wire callbacks between those components and runtime state
- Surface a single window/webview host interface to `WindowHandle`

## State ownership changes

### `Application`

Before:

- owns `WindowDriver`
- owns `WebviewDriver`

After:

- owns `BackendRuntime`

### `RuntimeWindowState`

Before:

- stores `WindowDriver*`
- stores `WebviewDriver*`

After:

- stores `std::shared_ptr<WindowHost>`

This removes direct platform driver knowledge from shared runtime state.

## Data flow

### Window creation

1. `Application::create_window(...)` validates public rules.
2. `Application` requests a runtime from `BackendFactory` if it does not already
   have one.
3. `BackendRuntime::create_window(...)` runs backend selection and initialization.
4. `LinuxX11BackendRuntime` calls `KernelResolver`.
5. `LinuxX11BackendRuntime` creates a `LinuxX11WindowHost`.
6. `LinuxX11WindowHost` creates and wires `WindowDriver` and `WebviewDriver`.
7. `Application` stores the returned `WindowHost` in `RuntimeWindowState`.
8. `WindowHandle` is returned.

### Window operations

1. User calls `WindowHandle::set_title(...)`.
2. `WindowHandle` validates `window_closed`.
3. `WindowHandle` calls `WindowHost::set_title(...)`.
4. `LinuxX11WindowHost` forwards to `WindowDriver`.

### Run loop

1. User calls `Application::run()`.
2. `Application` validates public runtime state.
3. `Application` calls `BackendRuntime::run(...)`.
4. `LinuxX11BackendRuntime` runs the GTK main loop through its owned host/runtime
   integration.

## Error handling

The refactor preserves existing external error behavior.

Errors that remain externally stable:

- `multiple_windows_unsupported`
- `window_closed`
- `invalid_state`
- `unsupported_by_backend`
- `engine_init_failed`

Internal backend-specific failures may be relocated, but should not change their
public code values unless the current code is clearly incorrect.

## Testing strategy

Keep the existing suite passing, then add focused tests for the new layer.

Required validation:

- All existing tests continue to pass.
- `Application` tests still verify single-window enforcement and lifecycle rules.
- `WindowHandle` tests still verify stable handle behavior after close.
- Add at least one unit test that verifies `Application` no longer requires
  concrete driver pointers in `RuntimeWindowState`.
- Add at least one unit/integration test that exercises backend creation through
  `BackendFactory`.

## Migration sequence

1. Add `BackendRuntime`, `WindowHost`, and `BackendFactory` abstractions.
2. Add Linux/X11 runtime and window host concrete implementations.
3. Move `Application` ownership from concrete drivers to `BackendRuntime`.
4. Move `WindowHandle` dispatch from concrete drivers to `WindowHost`.
5. Remove concrete driver pointers from `RuntimeWindowState`.
6. Connect `KernelResolver` into the runtime creation path.
7. Run full verification.

## Result

After this refactor, the public runtime is still Linux/X11-backed, but it is no
longer Linux/X11-shaped internally. That creates a real cross-platform runtime
skeleton instead of only a cross-platform public surface.
