#pragma once

#include <string>
#include <unordered_map>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <viewshell/types.h>

namespace viewshell {

struct RuntimeAppState {
  bool run_started = false;
  bool shutdown_started = false;
  int run_exit_code = 0;
  std::thread::id owner_thread = std::this_thread::get_id();
  std::deque<std::function<void()>> posted_tasks;
  std::mutex mutex;
  std::condition_variable cv;
  std::vector<std::string> logs;
};

struct RuntimeWindowState {
  bool has_window = false;
  bool is_closed = false;
  bool close_acknowledged = false;
  std::unordered_map<std::string, CommandHandler> command_registry;
};

} // namespace viewshell
