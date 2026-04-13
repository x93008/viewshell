#include "bridge/builtin_dispatcher.h"

namespace viewshell {

BuiltinDispatcher::BuiltinDispatcher(BuiltinContext context)
    : context_(std::move(context)) {}

namespace {

Result<Json> ok_result() { return Json{{"ok", true}}; }

Result<Json> void_to_json(Result<void> r) {
  if (!r) return tl::unexpected(r.error());
  return ok_result();
}

} // namespace

Result<Json> BuiltinDispatcher::dispatch(BuiltinOpcode opcode, const Json& args) {
  switch (opcode) {
    case BuiltinOpcode::SetTitle:
      return void_to_json(context_.set_title(args.value("title", "")));
    case BuiltinOpcode::Maximize:
      return void_to_json(context_.maximize());
    case BuiltinOpcode::Unmaximize:
      return void_to_json(context_.unmaximize());
    case BuiltinOpcode::Minimize:
      return void_to_json(context_.minimize());
    case BuiltinOpcode::Unminimize:
      return void_to_json(context_.unminimize());
    case BuiltinOpcode::Show:
      return void_to_json(context_.show());
    case BuiltinOpcode::Hide:
      return void_to_json(context_.hide());
    case BuiltinOpcode::Focus:
      return void_to_json(context_.focus());
    case BuiltinOpcode::SetSize:
      return void_to_json(context_.set_size(Size{args.value("width", 0), args.value("height", 0)}));
    case BuiltinOpcode::GetSize: {
      auto r = context_.get_size();
      if (!r) return tl::unexpected(r.error());
      return Json{{"width", r->width}, {"height", r->height}};
    }
    case BuiltinOpcode::SetPosition:
      return void_to_json(context_.set_position(Position{args.value("x", 0), args.value("y", 0)}));
    case BuiltinOpcode::GetPosition: {
      auto r = context_.get_position();
      if (!r) return tl::unexpected(r.error());
      return Json{{"x", r->x}, {"y", r->y}};
    }
    case BuiltinOpcode::SetBorderless:
      return void_to_json(context_.set_borderless(args.value("enabled", false)));
    case BuiltinOpcode::SetAlwaysOnTop:
      return void_to_json(context_.set_always_on_top(args.value("enabled", false)));
    case BuiltinOpcode::BeginDrag:
      return void_to_json(context_.begin_drag());
    case BuiltinOpcode::Close:
      return void_to_json(context_.close());
    case BuiltinOpcode::Capabilities:
      return tl::unexpected(Error{"unsupported_by_backend", "use TrustGate"});
  }
  return tl::unexpected(Error{"unknown_opcode", ""});
}

} // namespace viewshell
