#include <gtest/gtest.h>
#include "webview/x11_webview_driver.h"

TEST(WebviewDriver, rejects_unsupported_url_schemes) {
  EXPECT_FALSE(viewshell::WebviewDriver::is_allowed_url_scheme("file:///tmp/index.html"));
  EXPECT_FALSE(viewshell::WebviewDriver::is_allowed_url_scheme("data:text/html,<h1>hi</h1>"));
  EXPECT_TRUE(viewshell::WebviewDriver::is_allowed_url_scheme("https://example.com"));
  EXPECT_TRUE(viewshell::WebviewDriver::is_allowed_url_scheme("http://example.com"));
}
