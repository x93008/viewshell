#include <gtest/gtest.h>
#include <viewshell/application.h>

TEST(ApplicationCreate, rejects_invalid_config_before_runtime_boot) {
  viewshell::AppOptions options;
  options.bridge_timeout_ms = 0;
  auto result = viewshell::Application::create(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_config");
}
