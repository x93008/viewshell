#include "request_tracker.h"

namespace viewshell {

uint64_t RequestTracker::begin_request(int generation, int timeout_ms) {
  auto id = next_id_++;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  pending_[id] = {id, generation, deadline};
  return id;
}

Result<void> RequestTracker::complete_request(uint64_t id) {
  auto it = pending_.find(id);
  if (it == pending_.end()) {
    return tl::unexpected(Error{"request_not_found", ""});
  }
  pending_.erase(it);
  return {};
}

Error RequestTracker::fail_request(uint64_t id, const std::string& code) {
  auto it = pending_.find(id);
  if (it == pending_.end()) {
    return Error{"request_not_found", ""};
  }
  pending_.erase(it);
  return Error{code, ""};
}

std::unordered_map<uint64_t, Error> RequestTracker::fail_generation(
    int generation, const std::string& code) {
  std::unordered_map<uint64_t, Error> failed;
  for (auto it = pending_.begin(); it != pending_.end();) {
    if (it->second.generation == generation) {
      failed[it->first] = Error{code, ""};
      it = pending_.erase(it);
    } else {
      ++it;
    }
  }
  return failed;
}

std::unordered_map<uint64_t, Error> RequestTracker::expire_timed_out_requests() {
  auto now = std::chrono::steady_clock::now();
  std::unordered_map<uint64_t, Error> expired;
  for (auto it = pending_.begin(); it != pending_.end();) {
    if (now >= it->second.deadline) {
      expired[it->first] = Error{"bridge_timeout", ""};
      it = pending_.erase(it);
    } else {
      ++it;
    }
  }
  return expired;
}

Error RequestTracker::reject_all_unavailable() {
  auto count = pending_.size();
  pending_.clear();
  return Error{"bridge_unavailable", ""};
}

bool RequestTracker::has_pending() const {
  return !pending_.empty();
}

} // namespace viewshell
