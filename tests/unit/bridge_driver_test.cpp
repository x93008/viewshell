#include <gtest/gtest.h>
#include "bridge/x11_bridge_driver.h"
#include "bridge_driver_test_hooks.h"

TEST(BridgeDriver, signals_ready_reset_and_raw_message) {
  viewshell::BridgeDriver driver;
  bool ready = false;
  bool reset = false;
  std::string raw;
  driver.on_bridge_ready = [&] { ready = true; };
  driver.on_bridge_reset = [&] { reset = true; };
  driver.on_raw_message = [&](std::string_view msg) { raw = msg; };
  ASSERT_TRUE(viewshell::TriggerBridgeReadyForTest(driver));
  ASSERT_TRUE(viewshell::TriggerBridgeRawMessageForTest(driver, "hello"));
  ASSERT_TRUE(viewshell::TriggerBridgeResetForTest(driver));
  EXPECT_TRUE(ready);
  EXPECT_TRUE(reset);
  EXPECT_EQ(raw, "hello");
}

TEST(BridgeDriver, post_to_page_delivers_to_active_generation_only) {
  viewshell::BridgeDriver driver;
  ASSERT_FALSE(driver.post_to_page("before-ready"));
  ASSERT_TRUE(viewshell::TriggerBridgeReadyForTest(driver));
  ASSERT_TRUE(driver.post_to_page("ping"));
  ASSERT_EQ(viewshell::LastPostedMessageForTest(driver), "ping");
  ASSERT_TRUE(viewshell::TriggerBridgeResetForTest(driver));
  ASSERT_FALSE(driver.post_to_page("after-reset"));
}

TEST(BridgeDriver, attach_injects_bootstrap_and_treats_script_messages_as_ready) {
  viewshell::BridgeDriver driver;
  bool ready = false;
  std::string raw;
  driver.on_bridge_ready = [&] { ready = true; };
  driver.on_raw_message = [&](std::string_view msg) { raw = msg; };

  ASSERT_TRUE(viewshell::TriggerBridgeReadyForTest(driver));
  ASSERT_TRUE(viewshell::TriggerBridgeRawMessageForTest(driver, R"({"kind":"invoke","name":"app.ping","payload":{}})"));
  EXPECT_TRUE(ready);
  EXPECT_NE(raw.find("\"kind\":\"invoke\""), std::string::npos);
}
