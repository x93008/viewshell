#include <gtest/gtest.h>
#include "platform/linux_x11/trust_gate.h"

TEST(TrustGate, untrusted_remote_gets_no_bridge) {
  auto state = viewshell::TrustGate::classify(
      "https://untrusted.example", {"https://allowed.example"});
  EXPECT_EQ(state.mode, viewshell::TrustMode::NoBridge);
}

TEST(TrustGate, trusted_remote_gets_reduced_surface) {
  auto state = viewshell::TrustGate::classify(
      "https://allowed.example", {"https://allowed.example"});
  EXPECT_EQ(state.mode, viewshell::TrustMode::ReducedBridge);
}

TEST(TrustGate, local_app_gets_full_bridge) {
  auto state = viewshell::TrustGate::classify(
      "viewshell://app/index.html", {});
  EXPECT_EQ(state.mode, viewshell::TrustMode::FullBridge);
}

TEST(TrustGate, local_app_is_recognized) {
  EXPECT_TRUE(viewshell::TrustGate::is_local_app("viewshell://app/index.html"));
  EXPECT_FALSE(viewshell::TrustGate::is_local_app("https://example.com"));
}

TEST(TrustGate, reduced_bridge_masks_resource_protocol_and_webview_ops) {
  auto state = viewshell::TrustGate::classify(
      "https://allowed.example", {"https://allowed.example"});
  EXPECT_EQ(state.mode, viewshell::TrustMode::ReducedBridge);
  EXPECT_FALSE(state.effective_capabilities.webview.resource_protocol);
}

TEST(TrustGate, full_bridge_preserves_all_capabilities) {
  viewshell::Capabilities caps;
  caps.window.native_drag = true;
  caps.webview.resource_protocol = true;
  caps.bridge.invoke = true;
  caps.bridge.native_events = true;

  auto state = viewshell::TrustGate::classify(
      "viewshell://app/index.html", {});
  EXPECT_EQ(state.mode, viewshell::TrustMode::FullBridge);
}
