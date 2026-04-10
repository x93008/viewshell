#include <gtest/gtest.h>
#include "test_app_harness.h"

class RemoteUntrusted : public ::testing::Test {
protected:
  void SetUp() override {
    if (!viewshell::HasDisplay()) {
      GTEST_SKIP() << "No display available, skipping integration test";
    }
  }
  viewshell::TestAppHarness app;
};

TEST_F(RemoteUntrusted, launch_remote_succeeds) {
  ASSERT_TRUE(app.launch_remote("https://untrusted.example"));
}
