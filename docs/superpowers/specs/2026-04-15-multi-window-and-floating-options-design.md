# Multi-Window And Floating Options Design

Date: 2026-04-15
Status: Approved in chat, implementation in progress

## Goal

Lift the current single-window restriction and add the minimum window options
required to build a true multi-window floating-panel demo.

## Scope

- support arbitrary multiple windows per `Application`
- add floating-window relevant options to `WindowOptions`
- implement those options on Linux, Windows, and macOS where possible

## Initial options

- `show_in_taskbar` (default `true`)
- `resizable` (default `true`)

## Non-goals

- the floating demo itself in this step
- cross-window z-order orchestration beyond basic platform support
