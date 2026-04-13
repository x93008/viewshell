#include <gtest/gtest.h>
#include "bridge/invoke_bus.h"

using viewshell::Json;
using viewshell::InvokeBus;
using viewshell::Result;

TEST(InvokeBus, rejects_unknown_command) {
  InvokeBus bus;
  auto result = bus.dispatch("app.missing", nlohmann::json::object());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "command_not_found");
}

TEST(InvokeBus, dispatches_registered_command) {
  InvokeBus bus;
  bus.register_command("app.ping", [](const nlohmann::json& args) -> Result<nlohmann::json> {
    return nlohmann::json{{"pong", args.value("value", 0)}};
  });
  auto result = bus.dispatch("app.ping", nlohmann::json{{"value", 42}});
  ASSERT_TRUE(result);
  EXPECT_EQ((*result)["pong"], 42);
}

TEST(InvokeBus, delivers_native_events_and_off_is_idempotent) {
  InvokeBus bus;
  Json received;
  auto sub = bus.subscribe("native-ready", [&](const Json& payload) {
    received = payload;
  });
  ASSERT_TRUE(sub);
  EXPECT_TRUE(bus.emit("native-ready", nlohmann::json{{"ok", true}}));
  EXPECT_EQ(received["ok"], true);
  EXPECT_TRUE(sub->off());
  EXPECT_TRUE(sub->off());
}

TEST(InvokeBus, drop_subscriptions_clears_all) {
  InvokeBus bus;
  auto sub = bus.subscribe("native-ready", [](const Json&) { return; });
  ASSERT_TRUE(sub);
  bus.drop_subscriptions("bridge_reset");
  EXPECT_FALSE(bus.emit("native-ready", nlohmann::json{{"ok", true}}));
}
