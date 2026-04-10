#include <gtest/gtest.h>
#include "window_driver_test_hooks.h"
#include <platform/linux_x11/window_driver.h>
#include <gtk/gtk.h>

TEST(WindowDriver, applies_basic_window_operations) {
  gtk_init(nullptr, nullptr);

  viewshell::WindowDriver driver;
  viewshell::WindowOptions options;
  options.width = 640;
  options.height = 480;
  options.borderless = true;

  ASSERT_TRUE(driver.create(options));
  EXPECT_TRUE(driver.set_title("Viewshell"));
  EXPECT_TRUE(driver.set_size({800, 600}));
  EXPECT_TRUE(driver.set_position({20, 30}));
  EXPECT_TRUE(driver.get_position());
  EXPECT_TRUE(driver.get_size());
  EXPECT_TRUE(driver.set_borderless(false));
  EXPECT_TRUE(driver.set_always_on_top(true));
  EXPECT_TRUE(driver.maximize());
  EXPECT_TRUE(driver.unmaximize());
  EXPECT_TRUE(driver.minimize());
  EXPECT_TRUE(driver.unminimize());
  EXPECT_TRUE(driver.show());
  EXPECT_TRUE(driver.hide());
  EXPECT_TRUE(driver.focus());
  EXPECT_TRUE(driver.close());
}

TEST(WindowDriver, rejects_begin_drag_without_active_drag_context) {
  gtk_init(nullptr, nullptr);
  viewshell::WindowDriver driver;
  ASSERT_TRUE(driver.create({}));
  viewshell::DragContext drag{};
  drag.is_valid = false;
  auto result = driver.begin_drag(drag);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "invalid_drag_context");
  driver.close();
}

TEST(WindowDriver, emits_close_callback) {
  gtk_init(nullptr, nullptr);
  viewshell::WindowDriver driver;
  bool closed = false;
  driver.on_close = [&] { closed = true; };
  ASSERT_TRUE(driver.create({}));
  EXPECT_TRUE(driver.close());
  EXPECT_TRUE(closed);
}
