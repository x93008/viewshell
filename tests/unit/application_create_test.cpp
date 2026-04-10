#include <gtest/gtest.h>
#include <viewshell/application.h>
#include "public_types_test_api.h"

TEST(ApplicationCreate, rejects_invalid_config_before_runtime_boot) {
  viewshell::AppOptions options;
  options.bridge_timeout_ms = 0;
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}

TEST(ApplicationCreate, rejects_invalid_trusted_origin_strings) {
  viewshell::AppOptions options;
  options.trusted_origins = {"not-an-origin"};
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}

TEST(ApplicationCreate, normalizes_default_ports_and_deduplicates_origins) {
  viewshell::AppOptions options;
  options.trusted_origins = {
    "https://example.com:443",
    "https://example.com"
  };
  auto normalized = viewshell::detail::normalize_app_options_for_test(options);
  ASSERT_TRUE(normalized);
  EXPECT_EQ(normalized->trusted_origins.size(), 1u);
  EXPECT_EQ(normalized->trusted_origins.front(), "https://example.com");
}

TEST(ApplicationCreate, rejects_negative_bridge_timeout) {
  viewshell::AppOptions options;
  options.bridge_timeout_ms = -1;
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}
