# ViewShell

A C++ library for hosting frontend web pages in native windows using system WebView engines, similar to Tauri.

**Language:** C++17

## Requirements

- CMake 3.26+
- GCC 10+ (or compatible C++17 compiler)
- Conan 2 (`conanfile.txt` manages dependencies)
- GTK3, X11, WebKitGTK (system packages)

## Build

```bash
# install dependencies
conan install . -b missing

# configure
cmake --preset conan-debug --fresh

# build
cmake --build --preset conan-debug
```

## Dependencies

Managed via Conan 2 (`conanfile.txt`):

- nlohmann_json/3.11.3
- tl-expected/1.1.0
- gtest/1.14.0

System dependencies (pkg-config):

- GTK3, X11, WebKitGTK 4.1 (or 4.0 fallback)

## JavaScript Bridge

The Linux X11 backend currently injects a minimal bridge into the page:

```js
window.__viewshell.invoke(name, args) -> Promise
window.__viewshell.emit(name, payload) -> void
window.__viewshell.on(name, listener) -> unsubscribe function
window.__viewshell.off(name, listener) -> void
```

Current behavior:

- `invoke(...)` resolves or rejects from native `invoke_result` messages
- `emit(...)` is fire-and-forget
- `on/off` currently dispatches `native_event` messages inside the page

Current limitations:

- no JS-side timeout handling for unresolved invokes
- no typed event subscription protocol between JS and native yet
- bridge API is currently implemented for Linux X11 WebKitGTK only
