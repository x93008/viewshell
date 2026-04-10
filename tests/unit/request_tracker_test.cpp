#include <gtest/gtest.h>
#include "platform/linux_x11/request_tracker.h"

TEST(RequestTracker, invalidates_pending_requests_on_bridge_reset) {
  viewshell::RequestTracker tracker;
  auto id = tracker.begin_request(1);
  auto result = tracker.fail_generation(1, "bridge_reset");
  EXPECT_EQ(result.at(id).code, "bridge_reset");
}

TEST(RequestTracker, expires_requests_with_bridge_timeout) {
  viewshell::RequestTracker tracker;
  auto id = tracker.begin_request(1, 0);
  auto result = tracker.expire_timed_out_requests();
  ASSERT_FALSE(result.empty());
  EXPECT_EQ(result.at(id).code, "bridge_timeout");
}

TEST(RequestTracker, individual_fail_returns_correct_code) {
  viewshell::RequestTracker tracker;
  auto id = tracker.begin_request(1);
  auto err = tracker.fail_request(id, "window_closed");
  EXPECT_EQ(err.code, "window_closed");
  EXPECT_FALSE(tracker.has_pending());
}

TEST(RequestTracker, reject_all_returns_unavailable) {
  viewshell::RequestTracker tracker;
  tracker.begin_request(1);
  tracker.begin_request(1);
  auto err = tracker.reject_all_unavailable();
  EXPECT_EQ(err.code, "bridge_unavailable");
  EXPECT_FALSE(tracker.has_pending());
}

TEST(RequestTracker, complete_request_removes_from_pending) {
  viewshell::RequestTracker tracker;
  auto id = tracker.begin_request(1);
  EXPECT_TRUE(tracker.has_pending());
  auto result = tracker.complete_request(id);
  EXPECT_TRUE(result);
  EXPECT_FALSE(tracker.has_pending());
}
