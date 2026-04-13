#include <gtest/gtest.h>
#include "webview/resource_protocol.h"

TEST(ResourceProtocol, rejects_paths_outside_asset_root) {
  viewshell::ResourceProtocol protocol("/tmp/app");
  auto result = protocol.resolve("viewshell://app/../secret.txt");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}

TEST(ResourceProtocol, maps_missing_file_to_resource_not_found) {
  viewshell::ResourceProtocol protocol("/tmp/app");
  auto result = protocol.resolve("viewshell://app/missing.js");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_not_found");
}

TEST(ResourceProtocol, missing_entry_file_maps_to_resource_not_found) {
  auto result = viewshell::ResourceProtocol::from_entry_file(
      "tests/fixtures/local_app/missing.html", {});
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_not_found");
}

TEST(ResourceProtocol, falls_back_to_application_octet_stream_for_unknown_extension) {
  viewshell::ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("viewshell://app/blob.unknownext");
  ASSERT_TRUE(result);
  EXPECT_EQ(result->mime_type, "application/octet-stream");
}

TEST(ResourceProtocol, rejects_wrong_scheme) {
  viewshell::ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("file://app/index.html");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}

TEST(ResourceProtocol, rejects_wrong_host) {
  viewshell::ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("viewshell://other/index.html");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}

TEST(ResourceProtocol, resolves_index_under_viewshell_app_origin) {
  viewshell::ResourceProtocol protocol("tests/fixtures/local_app");
  auto result = protocol.resolve("viewshell://app/index.html");
  ASSERT_TRUE(result);
  EXPECT_EQ(result->mime_type, "text/html");
}

TEST(ResourceProtocol, rejects_entry_outside_explicit_asset_root) {
  viewshell::WindowOptions options;
  options.asset_root = "/tmp/app";
  auto result = viewshell::ResourceProtocol::from_entry_file(
      "/tmp/other/index.html", options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "resource_out_of_scope");
}

TEST(ResourceProtocol, reestablishes_inferred_scope_for_new_entry_file) {
  auto first = viewshell::ResourceProtocol::from_entry_file(
      "tests/fixtures/local_app/index.html", {});
  auto second = viewshell::ResourceProtocol::from_entry_file(
      "tests/fixtures/alt_app/index.html", {});
  ASSERT_TRUE(first);
  ASSERT_TRUE(second);
  EXPECT_NE(first->asset_root(), second->asset_root());
}

TEST(ResourceProtocol, preserves_nested_entry_path_under_viewshell_app_origin) {
  auto nested = viewshell::ResourceProtocol::from_entry_file(
      "tests/fixtures/local_app/settings/index.html", {});
  ASSERT_TRUE(nested);
  EXPECT_EQ(nested->entry_url(), "viewshell://app/settings/index.html");
}
