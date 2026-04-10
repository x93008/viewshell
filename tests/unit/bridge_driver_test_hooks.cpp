#include "bridge_driver_test_hooks.h"

namespace viewshell {

bool TriggerBridgeReadyForTest(BridgeDriver& driver) {
  driver.bridge_ready_ = true;
  if (driver.on_bridge_ready) driver.on_bridge_ready();
  return true;
}

bool TriggerBridgeRawMessageForTest(BridgeDriver& driver, std::string_view msg) {
  if (driver.on_raw_message) driver.on_raw_message(msg);
  return true;
}

bool TriggerBridgeResetForTest(BridgeDriver& driver) {
  driver.bridge_ready_ = false;
  driver.generation_++;
  if (driver.on_bridge_reset) driver.on_bridge_reset();
  return true;
}

std::string LastPostedMessageForTest(const BridgeDriver& driver) {
  return driver.last_posted_;
}

} // namespace viewshell
