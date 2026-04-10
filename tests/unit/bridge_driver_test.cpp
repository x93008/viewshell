#include <gtest/gtest.h>
#include "platform/linux_x11/bridge_driver.h"
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
