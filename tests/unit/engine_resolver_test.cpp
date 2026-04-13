#include <gtest/gtest.h>
#include <viewshell/options.h>
#include <viewshell/capabilities.h>
#include "webview/engine_resolver.h"
#include "engine_resolver_test_support.h"

namespace {

viewshell::Capabilities full_capabilities() {
  viewshell::Capabilities caps;
  caps.window.borderless = true;
  caps.window.transparent = true;
  caps.window.always_on_top = true;
  caps.window.native_drag = true;
  caps.webview.devtools = true;
  caps.webview.resource_protocol = true;
  caps.webview.script_eval = true;
  caps.bridge.invoke = true;
  caps.bridge.native_events = true;
  return caps;
}

viewshell::EngineResolver::ProbeFn missing_only_probe() {
  return [](std::string_view) -> viewshell::EngineResolver::ProbeResult {
    return {.library_found = false, .init_success = false, .required_probes_ok = false};
  };
}

viewshell::EngineResolver::ProbeFn failing_init_probe() {
  return [](std::string_view) -> viewshell::EngineResolver::ProbeResult {
    return {.library_found = true, .init_success = false, .required_probes_ok = false};
  };
}

viewshell::EngineResolver::ProbeFn no_devtools_probe() {
  auto caps = full_capabilities();
  caps.webview.devtools = false;
  return [caps](std::string_view) -> viewshell::EngineResolver::ProbeResult {
    return {.library_found = true, .init_success = true, .required_probes_ok = true, .capabilities = caps};
  };
}

viewshell::EngineResolver::ProbeFn fallback_success_probe() {
  return [](std::string_view candidate) -> viewshell::EngineResolver::ProbeResult {
    if (candidate == "libwebkit2gtk-4.1.so.0") {
      return {.library_found = true, .init_success = true, .required_probes_ok = false};
    }
    auto caps = full_capabilities();
    return {.library_found = true, .init_success = true, .required_probes_ok = true, .capabilities = caps};
  };
}

} // namespace

TEST(EngineResolver, rejects_non_webkit_required_engine) {
  viewshell::AppOptions options;
  options.require_engine = "webview2";
  auto result = viewshell::EngineResolver::resolve(options);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "engine_incompatible");
}

TEST(EngineResolver, default_resolve_supports_linux_webkit_runtime) {
  auto result = viewshell::EngineResolver::resolve({});
  ASSERT_TRUE(result);
  EXPECT_EQ(result->engine_id, "webkitgtk");
  EXPECT_TRUE(result->capabilities.webview.resource_protocol);
}

TEST(EngineResolver, uses_explicit_engine_path_first) {
  viewshell::AppOptions options;
  options.engine_path = "/tmp/libwebkit-custom.so";
  auto paths = viewshell::EngineResolver::candidate_paths(options);
  ASSERT_FALSE(paths.empty());
  EXPECT_EQ(paths.front(), "/tmp/libwebkit-custom.so");
}

TEST(EngineResolver, reports_not_found_with_attempted_candidates) {
  auto result = viewshell::ResolveWithProbeForTest(missing_only_probe());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "engine_not_found");
  EXPECT_FALSE(result.error().details["attempted_candidates"].empty());
  EXPECT_FALSE(result.error().details["failure_reasons"].empty());
}

TEST(EngineResolver, reports_engine_init_failed_when_probe_creation_fails) {
  auto result = viewshell::ResolveWithProbeForTest(failing_init_probe());
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, "engine_init_failed");
}

TEST(EngineResolver, reduces_optional_capabilities_when_devtools_missing) {
  auto result = viewshell::ResolveWithProbeForTest(no_devtools_probe());
  ASSERT_TRUE(result);
  EXPECT_FALSE(result->capabilities.webview.devtools);
}

TEST(EngineResolver, falls_back_to_second_candidate_after_first_required_probe_failure) {
  auto result = viewshell::ResolveWithProbeForTest(fallback_success_probe());
  ASSERT_TRUE(result);
  EXPECT_EQ(result->library_path, "libwebkit2gtk-4.0.so.37");
}
