#pragma once

#include "bridge/x11_bridge_driver.h"

namespace viewshell {

bool TriggerBridgeReadyForTest(BridgeDriver& driver);
bool TriggerBridgeRawMessageForTest(BridgeDriver& driver, std::string_view msg);
bool TriggerBridgeResetForTest(BridgeDriver& driver);
std::string LastPostedMessageForTest(const BridgeDriver& driver);

} // namespace viewshell
