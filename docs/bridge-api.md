# Bridge API

Viewshell currently exposes a Linux X11 WebKitGTK bridge as `window.__viewshell`.

## API

```js
window.__viewshell.invoke(name, args) -> Promise<any>
window.__viewshell.emit(name, payload) -> void
window.__viewshell.on(name, listener) -> unsubscribe function
window.__viewshell.off(name, listener) -> void
```

## Behavior

- `invoke(...)` sends a request to native and returns a real `Promise`
- successful native replies resolve with `payload`
- failed native replies reject with `{ code, message }`
- `emit(...)` sends a fire-and-forget event into native
- `on/off` sends subscribe and unsubscribe messages to native
- native only delivers `native_event` messages for events currently subscribed by the page

## Event delivery

Native events arrive in two ways:

- page-level listener dispatch through `window.__viewshell.on(...)`
- raw debug visibility through the `viewshell:message` DOM event used by the examples

## Current limits

- Linux X11 WebKitGTK only
- no JS timeout for stuck invoke calls
- no persistence of subscriptions across page reloads beyond bootstrap re-registration
- no typed schema validation for payloads

## Example playground

`examples/hello_viewshell/` demonstrates:

- invoke resolve (`app.ping`)
- invoke reject (`app.fail`)
- native-backed subscribe/unsubscribe for `native-ready`
- fire-and-forget emit and message log inspection
