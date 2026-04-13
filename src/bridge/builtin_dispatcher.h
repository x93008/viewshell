#pragma once

#include <string>
#include <functional>
#include <viewshell/types.h>
#include <viewshell/capabilities.h>

namespace viewshell {

struct BuiltinContext {
  std::function<Result<void>(std::string_view)> set_title;
  std::function<Result<void>()> maximize;
  std::function<Result<void>()> unmaximize;
  std::function<Result<void>()> minimize;
  std::function<Result<void>()> unminimize;
  std::function<Result<void>()> show;
  std::function<Result<void>()> hide;
  std::function<Result<void>()> focus;
  std::function<Result<void>(Size)> set_size;
  std::function<Result<Size>()> get_size;
  std::function<Result<void>(Position)> set_position;
  std::function<Result<Position>()> get_position;
  std::function<Result<void>(bool)> set_borderless;
  std::function<Result<void>(bool)> set_always_on_top;
  std::function<Result<void>()> begin_drag;
  std::function<Result<void>()> close;
};

enum class BuiltinOpcode {
  SetTitle,
  Maximize,
  Unmaximize,
  Minimize,
  Unminimize,
  Show,
  Hide,
  Focus,
  SetSize,
  GetSize,
  SetPosition,
  GetPosition,
  SetBorderless,
  SetAlwaysOnTop,
  BeginDrag,
  Close,
  Capabilities
};

class BuiltinDispatcher {
public:
  explicit BuiltinDispatcher(BuiltinContext context);
  Result<Json> dispatch(BuiltinOpcode opcode, const Json& args);

private:
  BuiltinContext context_;
};

} // namespace viewshell
