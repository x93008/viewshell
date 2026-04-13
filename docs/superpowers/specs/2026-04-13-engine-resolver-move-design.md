# Engine Resolver Move Design

Date: 2026-04-13
Status: Approved in chat, implementation in progress

## Goal

Move the X11/Linux web engine resolver out of `src/platform/x11/` and into the
webview subsystem as `src/webview/engine_resolver.*`.

## Rationale

`KernelResolver` is not really window-platform glue. Its responsibility is engine
discovery and capability probing for the webview layer, so it belongs closer to
webview concerns than platform adapters.

## Scope

- rename `kernel_resolver.*` to `engine_resolver.*`
- move it from `src/platform/x11/` to `src/webview/`
- rename type `KernelResolver` to `EngineResolver`
- update tests, CMake, and includes

## Non-goals

- changing resolver behavior
- making it cross-platform in this change
