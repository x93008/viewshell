#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <viewshell/types.h>

namespace viewshell {

struct PendingRequest {
  uint64_t id;
  int generation;
  std::chrono::steady_clock::time_point deadline;
};

class RequestTracker {
public:
  uint64_t begin_request(int generation, int timeout_ms = 5000);
  Result<void> complete_request(uint64_t id);
  Error fail_request(uint64_t id, const std::string& code);
  std::unordered_map<uint64_t, Error> fail_generation(int generation, const std::string& code);
  std::unordered_map<uint64_t, Error> expire_timed_out_requests();
  Error reject_all_unavailable();
  bool has_pending() const;

private:
  friend std::unordered_map<uint64_t, Error> ExpireTimedOutForTest(RequestTracker&);
  uint64_t next_id_ = 1;
  std::unordered_map<uint64_t, PendingRequest> pending_;
};

} // namespace viewshell
