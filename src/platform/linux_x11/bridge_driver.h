#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <optional>
#include <viewshell/types.h>

namespace viewshell {

class WebviewDriver;

bool TriggerBridgeReadyForTest(class BridgeDriver&);
bool TriggerBridgeRawMessageForTest(class BridgeDriver&, std::string_view);
bool TriggerBridgeResetForTest(class BridgeDriver&);
std::string LastPostedMessageForTest(const class BridgeDriver&);

class BridgeDriver {
public:
  Result<void> attach(WebviewDriver& webview);
  Result<void> post_to_page(std::string_view raw_message);

  std::function<void()> on_bridge_ready;
  std::function<void()> on_bridge_reset;
  std::function<void(std::string_view)> on_raw_message;

private:
  friend bool TriggerBridgeReadyForTest(BridgeDriver&);
  friend bool TriggerBridgeRawMessageForTest(BridgeDriver&, std::string_view);
  friend bool TriggerBridgeResetForTest(BridgeDriver&);
  friend std::string LastPostedMessageForTest(const BridgeDriver&);

  bool bridge_ready_ = false;
  int generation_ = 0;
  std::string last_posted_;
  WebviewDriver* webview_ = nullptr;
};

} // namespace viewshell
