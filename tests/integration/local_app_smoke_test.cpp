#include <gtest/gtest.h>
#include "test_app_harness.h"

class LocalAppSmoke : public ::testing::Test {
protected:
  void SetUp() override {
    if (!viewshell::HasDisplay()) {
      GTEST_SKIP() << "No display available, skipping integration test";
    }
  }
  viewshell::TestAppHarness app;
};

TEST_F(LocalAppSmoke, serves_index_under_viewshell_app_origin) {
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
}

TEST_F(LocalAppSmoke, second_load_file_reestablishes_active_asset_scope) {
  ASSERT_TRUE(app.launch_local("tests/fixtures/local_app/index.html"));
}
