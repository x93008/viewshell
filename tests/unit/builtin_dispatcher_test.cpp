#include <gtest/gtest.h>
#include "bridge/builtin_dispatcher.h"

namespace {

viewshell::BuiltinContext make_ok_context() {
  viewshell::BuiltinContext ctx;
  ctx.set_title = [](std::string_view) -> viewshell::Result<void> { return {}; };
  ctx.maximize = []() -> viewshell::Result<void> { return {}; };
  ctx.unmaximize = []() -> viewshell::Result<void> { return {}; };
  ctx.minimize = []() -> viewshell::Result<void> { return {}; };
  ctx.unminimize = []() -> viewshell::Result<void> { return {}; };
  ctx.show = []() -> viewshell::Result<void> { return {}; };
  ctx.hide = []() -> viewshell::Result<void> { return {}; };
  ctx.focus = []() -> viewshell::Result<void> { return {}; };
  ctx.set_size = [](viewshell::Size) -> viewshell::Result<void> { return {}; };
  ctx.get_size = []() -> viewshell::Result<viewshell::Size> { return viewshell::Size{100, 200}; };
  ctx.set_position = [](viewshell::Position) -> viewshell::Result<void> { return {}; };
  ctx.get_position = []() -> viewshell::Result<viewshell::Position> { return viewshell::Position{10, 20}; };
  ctx.set_borderless = [](bool) -> viewshell::Result<void> { return {}; };
  ctx.set_always_on_top = [](bool) -> viewshell::Result<void> { return {}; };
  ctx.begin_drag = []() -> viewshell::Result<void> { return {}; };
  ctx.close = []() -> viewshell::Result<void> { return {}; };
  return ctx;
}

}

TEST(BuiltinDispatcher, maps_set_always_on_top_opcode) {
  viewshell::BuiltinDispatcher dispatcher(make_ok_context());
  auto result = dispatcher.dispatch(
      viewshell::BuiltinOpcode::SetAlwaysOnTop,
      nlohmann::json{{"enabled", true}});
  EXPECT_TRUE(result);
}

TEST(BuiltinDispatcher, maps_set_title_opcode) {
  viewshell::BuiltinDispatcher dispatcher(make_ok_context());
  auto result = dispatcher.dispatch(
      viewshell::BuiltinOpcode::SetTitle,
      nlohmann::json{{"title", "Hello"}});
  EXPECT_TRUE(result);
}

TEST(BuiltinDispatcher, maps_get_size_opcode) {
  viewshell::BuiltinDispatcher dispatcher(make_ok_context());
  auto result = dispatcher.dispatch(
      viewshell::BuiltinOpcode::GetSize,
      nlohmann::json{});
  ASSERT_TRUE(result);
  EXPECT_EQ((*result)["width"], 100);
  EXPECT_EQ((*result)["height"], 200);
}
