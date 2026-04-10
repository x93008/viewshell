# Viewshell Design

Date: 2026-04-09
Status: Approved in chat, pending implementation planning

## Summary

Viewshell is a C++ library for hosting frontend applications inside native windows.
It owns native window management, system WebView loading, and native/frontend IPC.
It does not provide any frontend framework, build tooling, or frontend asset pipeline.

The library must:

- Expose native window operations such as move, maximize, minimize, borderless mode, resize, show/hide, title, focus, and always-on-top.
- Load frontend content from either a URL or built frontend files.
- Resolve and initialize a platform WebView engine automatically, while allowing callers to influence engine selection.
- Provide native/frontend IPC inspired by Tauri and Neutralino.
- Keep the minimal demo size under 10 MB by relying on system WebView engines instead of bundling Chromium.
- Preserve fast native window dragging comparable to Neutralino by using a native drag path instead of synthetic move loops.

## Confirmed Scope

### Platform scope

- Public API is cross-platform from day one.
- First complete implementation target is Linux on X11.
- Future backends are planned for Windows and macOS.

### Application scope

- The first release supports exactly one main window per application instance.
- `Application::create_window(...)` may be called once in the first release.
- A second window creation attempt returns `multiple_windows_unsupported`.

### Engine scope

- Only system WebView engines are supported.
- No CEF support.
- No external Chrome or Chromium `--app=` mode.
- On Linux, the first backend targets WebKitGTK.

### Frontend scope

- Viewshell does not own any frontend stack.
- The caller may use any frontend framework or no framework at all.
- The frontend is delivered as a URL or local built assets.
- Viewshell injects a small JavaScript bridge, but not a framework runtime.

### IPC scope

- No WebSocket transport.
- Built-in library APIs use a short native bridge path.
- User-defined commands and native events use a Tauri-like `invoke / event` model.
- All frontend-facing APIs return Promises, even if the native path is optimized internally.

## Goals

1. Provide a stable C++ host API for window and WebView control.
2. Provide a stable frontend bridge that works with arbitrary frontend stacks.
3. Keep built-in window operations lightweight and direct.
4. Keep user-defined IPC expressive, structured, and extensible.
5. Make backend capability differences explicit instead of hiding them behind fragile shims.
6. Keep the first deliverable small, debuggable, and realistic to ship.

## Non-goals

- Bundling a browser runtime.
- Building a frontend framework abstraction.
- Supporting multiple WebViews per window in the first release.
- Supporting Wayland in the first complete implementation.
- Simulating native drag by sending repeated move requests from JavaScript.

## Approaches Considered

### Approach A: Single generic invoke channel

Everything flows through one Tauri-style invoke system.

Pros:

- Simple conceptual model.
- Fewer internal code paths.
- Easy to document.

Cons:

- Built-in window operations inherit generic routing overhead.
- High-frequency or latency-sensitive built-ins become harder to optimize.
- Less aligned with the goal of keeping built-in APIs closer to Neutralino's short path.

### Approach B: Dual-path IPC

Built-in APIs use a specialized fast path. User-defined commands and native events use a generic invoke/event bus.

Pros:

- Best fit for the product goals.
- Keeps built-in window controls lightweight.
- Preserves a flexible user extension model.
- Allows a Promise-based public API without forcing all operations through the same internal route.

Cons:

- More internal structure than a single bus.
- Requires careful documentation so users understand which layer they are using.

### Approach C: Three transport classes

Separate fast path, normal built-in path, and user invoke path.

Pros:

- Maximum optimization flexibility.

Cons:

- Too much complexity for the first release.
- Harder to reason about and test.
- Not justified by current requirements.

### Decision

Use Approach B.

Viewshell will expose one consistent Promise-based frontend API, but implement two internal communication paths:

- a built-in fast path for library-provided native features
- a generic invoke/event path for user-defined app communication

## Architecture

Viewshell is split into five main layers.

### 1. Public host API

The public C++ API exposes application startup, window creation, WebView loading, and bridge registration.

First-release canonical ownership is:

- `Application` creates and owns windows
- `WindowHandle` is the main public handle
- each `Window` owns one primary WebView internally
- each `Window` exposes its bridge through `window.bridge()`

The host API does not expose a separate public `WebView` handle in the first release.
`window.load_url(...)` and `window.load_file(...)` are forwarding methods on `Window` that operate on the owned primary WebView.

Throughout the rest of this spec, `Window` refers to the public stable window handle abstraction, whose concrete API type is `WindowHandle` in the first release.
Throughout the rest of this spec, `Bridge` refers to the public stable bridge handle abstraction, whose concrete API type is `BridgeHandle` in the first release.

Representative host-side shape:

```cpp
auto app = viewshell::Application::create(config);
auto window = app.create_window(window_options);

window.load_url("https://example.com");
window.load_file("/path/to/dist/index.html");

window.set_size({800, 600});
window.set_borderless(true);

window.bridge().register_command("app.ping", handler);
window.bridge().emit("native-ready", payload);
```

### First-release host API contract

The first-release host API is non-throwing.
Public operations return `viewshell::Result<T>` or `viewshell::Result<void>`.
Errors are returned as values, not thrown across the public library boundary.

Representative signatures:

```cpp
viewshell::Result<Application> Application::create(const AppOptions& options);
viewshell::Result<WindowHandle> Application::create_window(const WindowOptions& options);
viewshell::Result<void> Application::post(std::function<void()> task);
viewshell::Result<int> Application::run();

viewshell::Result<BridgeHandle> WindowHandle::bridge();
viewshell::Result<void> WindowHandle::set_title(std::string_view title);
viewshell::Result<void> WindowHandle::maximize();
viewshell::Result<void> WindowHandle::unmaximize();
viewshell::Result<void> WindowHandle::minimize();
viewshell::Result<void> WindowHandle::unminimize();
viewshell::Result<void> WindowHandle::show();
viewshell::Result<void> WindowHandle::hide();
viewshell::Result<void> WindowHandle::focus();
viewshell::Result<void> WindowHandle::set_size(Size size);
viewshell::Result<Size> WindowHandle::get_size() const;
viewshell::Result<void> WindowHandle::set_position(Position position);
viewshell::Result<Position> WindowHandle::get_position() const;
viewshell::Result<void> WindowHandle::set_borderless(bool enabled);
viewshell::Result<void> WindowHandle::set_always_on_top(bool enabled);
viewshell::Result<void> WindowHandle::close();
viewshell::Result<void> WindowHandle::load_url(std::string_view url);
viewshell::Result<void> WindowHandle::load_file(std::string_view entry_file);
viewshell::Result<void> WindowHandle::reload();
viewshell::Result<void> WindowHandle::evaluate_script(std::string_view script);
viewshell::Result<void> WindowHandle::add_init_script(std::string_view script);
viewshell::Result<void> WindowHandle::open_devtools();
viewshell::Result<void> WindowHandle::close_devtools();
viewshell::Result<void> WindowHandle::on_page_load(PageLoadHandler handler);
viewshell::Result<void> WindowHandle::set_navigation_handler(NavigationHandler handler);
viewshell::Result<Capabilities> WindowHandle::capabilities() const;

using CommandHandler = std::function<viewshell::Result<Json>(const Json&)>;
viewshell::Result<void> BridgeHandle::register_command(std::string name, CommandHandler handler);
viewshell::Result<void> BridgeHandle::emit(std::string name, const Json& payload);

using PageLoadEvent = struct { std::string url; std::string stage; std::optional<std::string> error_code; };
using PageLoadHandler = std::function<void(const PageLoadEvent&)>;
using NavigationRequest = struct { std::string url; };
using NavigationHandler = std::function<NavigationDecision(const NavigationRequest&)>;
```

`WindowHandle` and the corresponding bridge handle are stable public handles owned by `Application`.
After native window closure they remain valid handle objects, but all operational methods return `window_closed`.

User-provided host callbacks are wrapped at the Viewshell boundary.

- if a registered command handler throws, Viewshell catches it and rejects the frontend request with `callback_threw`
- if a navigation handler throws, Viewshell treats the decision as `deny` and logs the failure
- if a page-load callback throws, Viewshell logs the failure and continues running
- if a task posted through `Application::post(...)` throws, Viewshell catches it, logs the failure, and continues running

### First-release configuration surface

The first release fixes configuration ownership as follows:

- `AppOptions.bridge_timeout_ms` configures request timeout behavior
- `AppOptions.trusted_origins` configures the allowlist for trusted remote bridge access
- `AppOptions.require_engine` configures a hard engine requirement
- `AppOptions.engine_path` provides an explicit engine library candidate path
- `WindowOptions.asset_root` overrides the inferred local asset root for `load_file(...)`

Other window presentation fields such as size, position, borderless, and always-on-top belong to `WindowOptions`.

`AppOptions.trusted_origins` is a list of normalized exact-origin strings in `scheme://host[:port]` form.
Invalid entries cause `Application::create(...)` to fail with `invalid_config`.
Default ports are normalized during parsing and duplicate origins are deduplicated after normalization.

`AppOptions.bridge_timeout_ms` must be a positive integer millisecond value.
`0` or negative values are rejected with `invalid_config`.

### 2. Public frontend bridge

Viewshell injects a thin JavaScript bridge into the loaded page.

Representative frontend shape:

```ts
await Viewshell.window.setSize({ width: 800, height: 600 });
await Viewshell.window.beginDrag();

await Viewshell.webview.loadFile("/index.html");

const result = await Viewshell.invoke("app.ping", { value: 1 });
const off = await Viewshell.on("native-ready", (event) => {
  console.log(event.payload);
});
```

The bridge is intentionally small. It exists to expose Viewshell, not to define an application model.

In the first release, the frontend bridge exposes five public surfaces:

- `Viewshell.window.*` for built-in window operations
- `Viewshell.webview.*` for built-in page operations bound to the primary WebView
- `Viewshell.invoke(name, payload)` for frontend-to-native custom commands
- `Viewshell.on(name, handler)` for native-to-frontend custom events
- `Viewshell.capabilities()` for frontend capability discovery

### First-release frontend window API contract

The canonical frontend window surface is:

- `Viewshell.window.setTitle(title: string): Promise<void>`
- `Viewshell.window.maximize(): Promise<void>`
- `Viewshell.window.unmaximize(): Promise<void>`
- `Viewshell.window.minimize(): Promise<void>`
- `Viewshell.window.unminimize(): Promise<void>`
- `Viewshell.window.show(): Promise<void>`
- `Viewshell.window.hide(): Promise<void>`
- `Viewshell.window.focus(): Promise<void>`
- `Viewshell.window.setSize(size: { width: number, height: number }): Promise<void>`
- `Viewshell.window.getSize(): Promise<{ width: number, height: number }>`
- `Viewshell.window.setPosition(pos: { x: number, y: number }): Promise<void>`
- `Viewshell.window.getPosition(): Promise<{ x: number, y: number }>`
- `Viewshell.window.setBorderless(enabled: boolean): Promise<void>`
- `Viewshell.window.setAlwaysOnTop(enabled: boolean): Promise<void>`
- `Viewshell.window.beginDrag(): Promise<void>`
- `Viewshell.window.close(): Promise<void>`

These map directly to the built-in fast-path window operations.

### First-release API matrix

| Feature | Host surface | Frontend surface |
|---|---|---|
| Window controls | `Window` | `Viewshell.window.*` |
| Primary WebView content loading | `Window` forwarding to owned WebView | `Viewshell.webview.loadUrl(...)`, `Viewshell.webview.loadFile(...)`, `Viewshell.webview.reload()` |
| Page lifecycle callbacks | `Window` | none |
| Navigation policy callback | `Window` | none |
| Custom command registration | `window.bridge()` | `Viewshell.invoke(...)` |
| Native-to-web custom events | `window.bridge()` | `Viewshell.on(...)` |
| Capability discovery | `Window` | `Viewshell.capabilities()` |

### First-release trust gates for frontend APIs

| Frontend API | Trusted local `viewshell://app/` | Trusted remote origin | Untrusted remote origin |
|---|---|---|---|
| `Viewshell.window.*` | yes | yes | no bridge |
| `Viewshell.webview.*` | yes | rejects with `operation_not_allowed` | no bridge |
| `Viewshell.invoke(...)` | yes | yes | no bridge |
| `Viewshell.on(...)` | yes | yes | no bridge |
| `Viewshell.capabilities()` | yes | yes | no bridge |

### Host execution model

The first release uses one Viewshell main loop thread per application.

- window callbacks run on the Viewshell main loop thread
- navigation handlers run on the Viewshell main loop thread
- user command handlers registered through `BridgeHandle::register_command(...)` run on the Viewshell main loop thread

Public host methods on `Application`, `WindowHandle`, and `BridgeHandle` must also be called on the Viewshell main loop thread after application creation.
Cross-thread host calls are rejected with `wrong_thread` in the first release rather than being marshalled automatically.

`Application::post(...)` is the only cross-thread-safe exception in the first release.
It is valid only after `run()` has started and before shutdown begins.
If called before `run()` or after shutdown starts, it returns `invalid_state`.

Command handlers in the first release are synchronous and must return a result directly.
Long-running host work is out of scope for the first release and should be deferred to a future async command model.

### Application lifecycle contract

The first-release lifecycle is:

- `Application::create(...)` establishes Viewshell ownership on the caller thread
- `app.run()` enters the Viewshell main loop on that same thread and blocks until shutdown
- exactly one main window must be created before `run()` in the first release
- `create_window(...)` is not valid after `run()` has started in the first release
- the first release supports one main window only
- when that window closes, Viewshell cancels in-flight requests, destroys the bridge, destroys the window, and exits the main loop
- after the main window closes, `create_window(...)` cannot succeed again in the same application instance

The first release does not define a close-interception API.
Closing the only window begins shutdown immediately.

Before `run()`, callers may perform startup configuration work on the main thread, including:

- `create_window(...)`
- initial `Window` configuration such as loading content or setting size and borderless mode
- `BridgeHandle::register_command(...)`

Before `run()`, `BridgeHandle::emit(...)` is rejected with `bridge_unavailable` because no active page bridge exists yet.
After `run()` begins, the same API family remains valid on the Viewshell main loop thread until window shutdown.

Calling `run()` without first creating the required main window fails with `invalid_state`.

`WindowHandle::close()` and `Viewshell.window.close()` resolve when native close acknowledgement has been received and shutdown has started.
Their own completion is not converted into `window_closed` by the general teardown cancellation rules.

`close()` on host or frontend resolves when native close acknowledgement has been received and shutdown has started.
Subsequent operations on the same stable handles return `window_closed`.

### 3. Built-in bridge

This bridge carries library-provided operations such as window state changes, sizing, dragging, content loading, and state queries.

It uses:

- compact numeric opcodes rather than string command routing
- direct dispatch to built-in controllers
- request IDs only where a response is required
- no WebSocket layer

### 4. Generic invoke/event bus

This bridge carries:

- user-defined commands
- native-to-web events

This layer uses structured message envelopes and a stable error model.

### 5. Backend layer

The backend is divided into platform and engine responsibilities.

- `WindowDriver` owns native window operations.
- `WebviewDriver` owns engine initialization and page hosting.
- `BridgeDriver` connects the WebView's native messaging API to Viewshell's built-in and generic bridge layers.
- `KernelResolver` selects and initializes the best available engine implementation.

### Bridge ownership contract

The first-release bridge stack is split into clear units:

- public `Bridge`: host-facing facade for command registration and native event emission
- `BridgeDriver`: transport attachment to the current page generation and raw message I/O
- `BuiltinDispatcher`: fixed-opcode dispatch for built-in APIs
- `InvokeBus`: string-command dispatch for user `invoke(...)`
- `RequestTracker`: request IDs, timeout bookkeeping, pending-request invalidation, and bridge-generation bookkeeping
- `TrustGate`: origin classification and API-surface gating before dispatch

Ownership rules:

- `BridgeDriver` does not own command registrations or trust policy
- `RequestTracker` owns request IDs and timeout state for both built-in and `invoke` requests
- `TrustGate` decides whether a page receives no bridge, a reduced bridge, or the full local bridge surface
- `BuiltinDispatcher` and `InvokeBus` consume messages only after `TrustGate` approval

Representative internal contracts are:

- `WindowDriver::create(WindowOptions) -> NativeWindowHandle`
- `WindowDriver::set_size(...)`, `set_position(...)`, `set_borderless(...)`, `begin_drag(DragContext)`
- `WindowDriver` callbacks: `on_close`, `on_resize`, `on_focus`
- `WebviewDriver::attach(NativeWindowHandle, WebviewOptions)`
- `WebviewDriver::navigate(url)`, `reload()`, `evaluate_script(...)`, `add_init_script(...)`
- `WebviewDriver` callbacks: `on_page_load`, `on_bridge_ready`, `on_bridge_reset`, `on_raw_message`
- `BridgeDriver::post_to_page(raw_message)` for active bridge generations only
- `KernelResolver::resolve(AppOptions) -> Result<ResolvedEngine>` where `ResolvedEngine` includes `engine_id`, `library_path`, and the accepted capability baseline
- `BuiltinDispatcher::dispatch(opcode, args, context) -> Result<Json>`
- `InvokeBus::dispatch(command, payload, context) -> Result<Json>`
- `RequestTracker::begin_request(...)`, `complete_request(...)`, `fail_generation(...)`
- `TrustGate::classify(origin) -> no_bridge | reduced_bridge | full_bridge`

### Backend composition contract

The first-release backend contract is:

- `WindowDriver` creates and owns the native window handle
- `WebviewDriver` is created with an attachment target supplied by `WindowDriver`
- `WebviewDriver` binds the primary WebView into that native window target
- `BridgeDriver` registers native script-message handlers on the `WebviewDriver`
- resize, focus, and close notifications flow from `WindowDriver` to `WebviewDriver`
- bridge resets and page lifecycle changes flow from `WebviewDriver` to `BridgeDriver`

`WindowDriver` remains the owner of native window lifecycle.
`WebviewDriver` owns the engine object attached to that window.
`BridgeDriver` owns transport attachment for the active bridge generation.

## Object Model

### Application

`Application` owns process-level setup and the event loop.

Responsibilities:

- startup configuration
- backend initialization
- window creation
- shutdown coordination

### Window

`Window` owns native host window state.

Responsibilities:

- title
- size and position
- maximize/minimize/show/hide/focus
- borderless and always-on-top
- close
- native drag entry points

### WebView

`WebView` owns web content rendering inside a single window.

In the first release, `WebView` is an internal backend unit rather than a first-class public host object.
Its responsibilities still matter architecturally, but callers interact with it through `Window`.

Responsibilities:

- loading a URL
- loading frontend files
- navigation and reload
- initialization scripts
- script evaluation
- page lifecycle events
- resource and protocol hooks

### Bridge

`Bridge` is bound to the active WebView and exposes:

- built-in API dispatch
- user command registration
- event emission

In the first release, `Bridge` is not an independently created public object.
It is reached through `window.bridge()` because each public `Window` owns one primary WebView and one bridge bound to that WebView.

Because the first release is single-window only, custom command registration and native event emission are scoped to that one main window.

### First-release simplification

Each `Window` owns exactly one primary `WebView` in the first release.

This keeps the lifecycle model simple and focused on the core goal: render one frontend application inside one native window with robust window control and IPC.

As a result, the first implementation plan should treat `Window` as the public facade over:

- native window state
- one owned primary WebView
- one bridge bound to that WebView

## Window API Design

### Core window operations

The first release guarantees these operations on Linux X11:

- `set_title`
- `maximize`
- `unmaximize`
- `minimize`
- `unminimize`
- `show`
- `hide`
- `focus`
- `set_size`
- `get_size`
- `set_position`
- `get_position`
- `set_borderless`
- `set_always_on_top`
- `close`

### Dragging semantics

Dragging must not be implemented by repeatedly calling `set_position` from frontend pointer events.

There is no public `WindowHandle::begin_drag()` in the first release.
Native drag is exposed through the frontend bridge as `Viewshell.window.beginDrag()`.

Instead:

- `begin_drag` is a dedicated built-in operation
- the frontend triggers it once from a drag-start action
- the native window system takes over the drag operation

This mirrors the behavior that makes Neutralino dragging feel responsive.

`set_position` remains available, but only for programmatic positioning, snap animations, or deliberate window choreography.

`begin_drag()` requires an active native drag context in the first release.
On Linux X11 this means an active primary-button gesture originating from the current window.
If that context is unavailable, `begin_drag()` fails with `invalid_drag_context`.

For frontend-triggered drags, the bridge transport captures the current input gesture metadata and forwards it through `BridgeDriver` to `WindowDriver::begin_drag(DragContext)`.

## WebView API Design

### Core content operations

The first release provides:

- `load_url`
- `load_file`
- `reload`
- `evaluate_script`
- `add_init_script`
- `open_devtools` if supported
- `close_devtools` if supported
- page load notifications
- navigation hooks

These are public host-side `Window` methods that forward to the owned primary WebView.
On the frontend side, the built-in WebView surface is:

- `Viewshell.webview.loadUrl(url)`
- `Viewshell.webview.loadFile(path)`
- `Viewshell.webview.reload()`

`Viewshell.webview.loadFile(path)` only accepts app-relative virtual paths within the already configured local asset scope, for example `/index.html` or `/settings/index.html`.
It does not accept arbitrary filesystem paths and cannot change `asset_root`.
It is only available for trusted local content running under `viewshell://app/`.

`add_init_script(...)` registers a script that runs on every trusted top-level document load of the primary WebView.
Execution order in the first release is:

1. trust gate decision for the target page
2. Viewshell bridge bootstrap for trusted pages
3. user-provided init scripts
4. page-authored application scripts

Init scripts do not run for untrusted remote pages.
Multiple `add_init_script(...)` registrations append in registration order.
Duplicate scripts are allowed.
Init scripts remain registered for the lifetime of the window and there is no removal API in the first release.

### Content loading model

Viewshell supports two content sources:

1. external or local URLs
2. local frontend build artifacts

For local assets, the preferred first-release approach is a native resource serving path integrated with the WebView backend, not a separate WebSocket or embedded application server.

### `load_url` scheme policy

In the first release, `load_url(...)` only accepts `http://` and `https://` URLs.

- `file://` is not supported in the first release and maps to `unsupported_url_scheme`
- `viewshell://app/` is not a valid `load_url(...)` target; local app content is established only through `load_file(...)`
- custom local content should be loaded through `load_file(...)`
- non-HTTP schemes other than the internal `viewshell://app/` resource origin are unsupported in the first release and map to `unsupported_url_scheme`

### `load_file` semantics

`load_file` takes an entry file path, for example `/path/to/dist/index.html`.

First-release behavior is defined as follows:

- the asset root defaults to the parent directory of the entry file
- callers may override the asset root explicitly through `WindowOptions.asset_root`
- if `WindowOptions.asset_root` is set and the entry file is outside that root, `load_file(...)` fails with `resource_out_of_scope`
- relative frontend asset requests resolve within that asset root
- any request that escapes the asset root via path traversal is rejected with `resource_out_of_scope`
- missing files map to `resource_not_found`
- MIME type is resolved from file extension, with `application/octet-stream` fallback
- SPA history fallback is out of scope for the first release and is not enabled by default

When `asset_root` is inferred rather than explicitly configured, each successful host-side `load_file(...)` call re-establishes the active local asset scope from the new entry file's parent directory.
Frontend `Viewshell.webview.loadFile(...)` stays within that currently active local asset scope.

This keeps local asset serving deterministic and testable for the initial implementation.

### Local content origin model

The first release serves local frontend assets under a dedicated trusted origin: `viewshell://app/`.

Rules:

- `load_file("/path/to/dist/index.html")` resolves to a trusted top-level document such as `viewshell://app/index.html`
- relative frontend assets resolve under the same `viewshell://app/` origin
- the native resource handler enforces `asset_root` scope restrictions before serving bytes
- bridge injection for local assets is keyed to this trusted local origin

The first release only injects the bridge and init scripts into the trusted top-level document.
Subframes and iframes do not receive Viewshell bridge injection or user init scripts, regardless of their origin.

This gives the local-content trust model one concrete, testable contract.

### Page lifecycle and navigation APIs

Because the primary WebView is owned through `Window` in the first release, page lifecycle and navigation APIs live on `Window`.

- `window.on_page_load(handler)` receives `{ url, stage, error_code? }` where `stage` is `started`, `finished`, or `failed`
- `window.set_navigation_handler(handler)` receives `{ url }` and returns `allow` or `deny`

The navigation handler is a blocking policy hook for top-level navigations of the primary WebView.
There is no separate frontend API for navigation hooks in the first release.

`window.on_page_load(handler)` accumulates handlers in registration order for the lifetime of the window.
`window.set_navigation_handler(handler)` installs a single active navigation policy handler; calling it again replaces the previous handler.
If navigation is denied, the navigation does not commit and no page-load callback is emitted for that denied request.

Navigation failure semantics are:

- if the installed navigation handler returns `deny`, the initiating `load_url(...)`, `load_file(...)`, `Viewshell.webview.loadUrl(...)`, or `Viewshell.webview.loadFile(...)` call fails with `navigation_denied`
- if `load_url(...)` or `load_file(...)` cannot submit the navigation request at all, the call returns `navigation_failed`
- if submission succeeds but the page later fails to load, the initiating call remains successful and `on_page_load` emits a `failed` stage with `error_code: "navigation_failed"`
- successful loads emit `started` followed by `finished`

Navigation supersession semantics are:

- starting a new top-level navigation or `reload()` while another top-level navigation is still in flight supersedes the older navigation
- the superseded navigation emits `on_page_load` with `stage: "failed"` and `error_code: "navigation_superseded"`
- the later navigation becomes the only active navigation tracked by the current page generation

## IPC Design

### Built-in fast path

Built-in operations are represented by a compact native envelope.

Representative shape:

```text
{ op: 7, req: 42, args: [...] }
```

Properties:

- fixed opcode table
- direct built-in dispatch
- minimal parsing overhead
- Promise resolution for all public frontend APIs

Setter-style operations resolve when the native UI thread has executed the action.
Getter-style operations resolve with a structured response payload.

### User invoke path

User-defined commands use a Tauri-like message model.

Representative shape:

```text
{
  kind: "invoke",
  id: 42,
  command: "app.ping",
  payload: { "value": 1 }
}
```

Properties:

- string command names
- structured payloads
- request/response lifecycle
- native-to-web event emission and subscription support

Frontend-to-native custom event emission is not part of the first release.
If the frontend needs to send data to native code, it uses `invoke`.

Channel-style streaming is also deferred beyond the first release.

### Event subscription contract

`Viewshell.on(name, handler)` returns `Promise<off>` where `off` is an idempotent unsubscribe function.

First-release behavior is:

- subscribing succeeds only when the bridge is available
- calling `off()` removes the subscription if it is still active
- calling `off()` after removal or after bridge teardown is a no-op
- all frontend subscriptions are dropped automatically when the page reloads, the bridge resets, or the window closes

### Command registration and emit contract

`Bridge::register_command(name, handler)` installs a synchronous handler for the active window bridge.

- registering a new unique name succeeds
- registering the same name twice returns `command_already_registered`
- command registrations are window-scoped and survive page reloads or bridge resets
- command registrations are rebound automatically when a new bridge generation becomes active
- command registrations are cleared when the window is destroyed

`Bridge::emit(name, payload)` is non-queueing in the first release.

- if the page is loaded and the bridge is active, the event is delivered to the current page generation
- if the bridge is not yet active, `emit(...)` returns `bridge_unavailable`
- if the bridge resets before delivery completes, `emit(...)` returns `bridge_reset`
- if the window is already closed, `emit(...)` returns `window_closed`

For the first release, `emit(...)` success means the event was handed off to the currently active bridge transport for the current page generation.
It does not wait for JavaScript-side handler execution or acknowledgement.

### Transport

The first Linux backend will use the WebView engine's native script message bridge rather than WebSocket.

That transport is reused by both layers, but the built-in layer and the generic layer remain logically separate.

### Promise contract

All frontend-visible APIs return Promises.

This gives the frontend one consistent calling model while still allowing internal performance specialization.

### Request lifecycle and teardown

Every in-flight frontend request is bound to:

- the owning window
- the current bridge generation for that window

First-release teardown behavior is:

- if the request succeeds before teardown, its Promise resolves normally
- if the request exceeds the configured deadline, its Promise rejects with `bridge_timeout`
- if the window closes before completion, its Promise rejects with `window_closed`
- if a page reload, top-level navigation, or bridge reinitialization invalidates the current bridge generation, its Promise rejects with `bridge_reset`
- if a call is attempted after the bridge is unavailable, it rejects with `bridge_unavailable`

These rules apply to both built-in requests and user `invoke` requests.

The timeout deadline is configured through `AppOptions.bridge_timeout_ms`.
The first-release default is `5000` milliseconds and it is shared by both built-in requests and user `invoke` requests.

## Capability Model

Viewshell exposes explicit backend capabilities instead of pretending every backend supports the same features.

Representative capabilities include:

- `borderless`
- `transparent`
- `always_on_top`
- `native_drag`
- `devtools`
- `resource_protocol`
- `script_eval`
- `native_events`

Host-side and frontend-side capability discovery are both supported.

- `window.capabilities()` on the host side, returning raw grouped window and primary-WebView backend capabilities
- `await Viewshell.capabilities()` on the frontend side, returning effective capabilities for the current page after backend support and trust gating are both applied

The capabilities payload is normative and all keys are always present.

Representative shape:

```json
{
  "window": {
    "borderless": true,
    "transparent": false,
    "always_on_top": true,
    "native_drag": true
  },
  "webview": {
    "devtools": true,
    "resource_protocol": true,
    "script_eval": true
  },
  "bridge": {
    "invoke": true,
    "native_events": true
  }
}
```

Unsupported capabilities are reported as `false` rather than being omitted.

If a capability is unavailable:

- capability discovery reports `false`
- calls return a structured `unsupported_by_backend` error

The first Linux X11 release treats these capabilities as guaranteed:

- window move and size control
- maximize and minimize
- borderless mode
- always-on-top
- content loading from URL or file
- built-in fast-path window APIs
- user `invoke`
- native-to-web events
- native drag
- script evaluation
- local resource protocol

## Engine Resolution and Compatibility

### Resolution inputs

The resolver reads these inputs from `AppOptions`:

- `require_engine`
- `engine_path`

Rules:

- `require_engine` enforces a hard selection requirement
- `engine_path` prepends a user-supplied candidate path

### Candidate evaluation

Linux X11 first-release candidates focus on the WebKitGTK family.

The deterministic first-release candidate order is:

1. `AppOptions.engine_path`, if provided
2. `libwebkit2gtk-4.1.so.0`
3. `libwebkit2gtk-4.0.so.37`

If `AppOptions.require_engine` is set, the first release only accepts `webkitgtk`; any other required engine value fails immediately with `engine_incompatible`.

For each candidate:

1. attempt to load the shared library
2. resolve required symbols
3. resolve optional symbols
4. initialize a probe instance if needed
5. accept, partially accept, or reject the candidate

### Required vs optional symbols

Required features include:

- WebView creation
- page loading
- script execution
- script message bridge
- init-script injection

Optional features include:

- devtools
- some advanced diagnostics

If a required feature is missing, Viewshell skips the candidate and tries the next one.
If only optional features are missing, Viewshell accepts the candidate and reports reduced capabilities.

The first release treats a candidate as accepted only if all required features for the local trusted-content path are present.
Partial acceptance means optional capability reduction only; it never means missing bridge, navigation, or init-script functionality.

### Failure reporting

Resolver failures use structured startup errors:

- `engine_not_found`
- `engine_incompatible`
- `engine_init_failed`

Errors include attempted candidates and failure reasons so users can debug deployment issues.

## Error Model

Frontend and host-visible errors use a stable structured format.

Host-side `viewshell::Result<T>` carries the same stable error codes as frontend Promise rejections.
The host API therefore has one shared error vocabulary across startup, window control, content loading, and IPC.

Representative shape:

```json
{
  "code": "unsupported_by_backend",
  "message": "The current backend does not support devtools",
  "details": {
    "backend": "webkitgtk-x11",
    "feature": "devtools"
  }
}
```

Representative error codes:

- `unsupported_by_backend`
- `window_not_ready`
- `webview_not_ready`
- `navigation_denied`
- `navigation_failed`
- `navigation_superseded`
- `bridge_timeout`
- `bridge_reset`
- `bridge_unavailable`
- `window_closed`
- `multiple_windows_unsupported`
- `wrong_thread`
- `callback_threw`
- `command_already_registered`
- `command_not_found`
- `operation_not_allowed`
- `invalid_drag_context`
- `invalid_payload`
- `invalid_config`
- `invalid_state`
- `resource_not_found`
- `resource_out_of_scope`
- `unsupported_url_scheme`
- `engine_not_found`
- `engine_incompatible`
- `engine_init_failed`

Normative trigger points for selected shared error codes are:

- `window_not_ready`: a frontend built-in window call arrives before the native window is ready to service it
- `webview_not_ready`: a webview operation arrives before the primary WebView is attached and ready
- `command_not_found`: `Viewshell.invoke(...)` targets a command name that has not been registered on the active window bridge
- `invalid_payload`: a built-in or user `invoke(...)` payload does not match the required shape or cannot be decoded

## Security Model

The first release keeps security modest but explicit.

- Built-in operations are only callable through the injected bridge.
- Local asset loading and custom protocols are constrained to the application's configured scope.
- No WebSocket server is exposed.

### Trust policy

The first release distinguishes trusted local content from untrusted remote content.

These trust gates apply to frontend-injected bridge APIs only.
Host-side privileged APIs such as `Window::evaluate_script(...)`, `Window::open_devtools()`, and `Window::close_devtools()` are controlled by host code and are not blocked by page trust classification, though normal backend capability checks still apply.

#### Local frontend assets

Locally loaded frontend assets are trusted by default.

They receive:

- the built-in Viewshell bridge
- user `invoke`
- native-to-web events
- configured local resource access

#### Remote URLs

Remote URLs are untrusted by default.

Default remote behavior:

- page loads are allowed if the host requested them
- the Viewshell bridge is not injected
- built-in APIs are unavailable
- user `invoke` is unavailable
- native-to-web events are unavailable

#### Trusted remote origins

Hosts may opt in specific remote origins through explicit configuration such as `trusted_origins`.

Only explicitly trusted origins may receive the injected bridge and participate in built-in APIs or user `invoke`.
This keeps the default model safe while still allowing controlled remote integrations.

Trusted remote origins receive a reduced built-in surface.
They may use `Viewshell.window.*`, `Viewshell.invoke(...)`, `Viewshell.on(...)`, and `Viewshell.capabilities()`.
They may not use any `Viewshell.webview.*` API in the first release.
Any `Viewshell.webview.*` call from a trusted remote bridge context rejects with `operation_not_allowed`.

If a backend cannot supply trustworthy origin metadata for a remote page, trusted-remote mode is unavailable on that backend and remote pages remain untrusted.

First-release matching rules for `trusted_origins` are intentionally strict:

- exact origin matching only
- scheme, host, and effective port must all match
- wildcard patterns are not supported
- bridge injection for remote pages is decided against the final committed top-level origin after redirects
- if the final committed origin is not trusted, the page remains untrusted even if an earlier redirect source was trusted

The security model is intentionally simpler than full Tauri policy machinery in the first release, but the invoke/event design should not block later permission expansion.

## First Implementation Target

### Linux X11 backend

First release backend:

- `X11WindowDriver`
- `WebKitGtkWebviewDriver`
- native script message bridge for built-ins and user IPC

Rationale:

- aligns with the first complete implementation target
- keeps binary size small by relying on system libraries
- supports native window ownership and native dragging
- avoids the complexity of dual Linux window-system support in the first version

## Testing Strategy

### Unit tests

Unit tests cover:

- builtin opcode mapping
- invoke/event envelope encoding and decoding
- Promise request correlation
- resolver candidate ordering and fallback rules
- capability exposure
- structured error generation
- trusted vs untrusted origin policy decisions

### Integration tests

Integration tests cover:

- create a window
- load a local frontend file
- load a URL
- call built-in window APIs from the frontend
- call user-defined commands from the frontend
- emit native events to the frontend
- verify capability queries
- verify untrusted remote URLs do not receive the bridge
- verify configured trusted origins do receive the bridge

Linux CI can use Xvfb for headless integration runs where appropriate.

### Dragging validation

Dragging is validated separately from normal movement.

- one demo validates `begin_drag` with a borderless window
- one demo validates `set_position` for snap or animation behavior
- `set_position` is never treated as the primary drag mechanism

## Demo and Acceptance Criteria

The first shippable demo must satisfy all of the following:

1. create a native window and load frontend content from local files
2. call built-in window APIs from JavaScript through the injected bridge
3. call user-defined native commands from JavaScript through `invoke`
4. emit native events back to JavaScript
5. support borderless mode and native drag on Linux X11
6. keep the stripped release demo under 10 MB
7. keep remote URLs untrusted by default unless explicitly allowlisted

For size verification, the measured artifact is the release demo output directory containing:

- the stripped demo executable
- shipped HTML, CSS, and JavaScript assets

System libraries provided by the target machine are not counted.
The acceptance check is `du -sb <demo-output-dir>` less than `10485760` bytes.

## Implementation Notes for Planning

Suggested implementation order:

1. core application and window lifecycle skeleton
2. Linux X11 window backend
3. WebKitGTK WebView backend
4. built-in fast-path bridge
5. user invoke and native-event bridge
6. local asset loading path
7. trusted-origin policy and bridge gating
8. capability reporting and resolver diagnostics
9. demo app and integration coverage

## Open Items Deferred Intentionally

These are deliberately postponed beyond the first design and implementation slice:

- Wayland backend
- Windows backend
- macOS backend
- multiple WebViews per window
- deeper permission system
- browser-engine family expansion beyond system engines
- channel-style streaming
- frontend-to-native custom event emission separate from invoke

## Final Decision

Viewshell will be a cross-platform C++ library with a Linux X11 first implementation, system-engine-only strategy, dual-path IPC, Promise-based frontend API, explicit backend capabilities, and native drag semantics.

This design keeps the first release small and practical while preserving a clean path to future Windows and macOS backends.
