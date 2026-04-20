#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace viewshell {

// Singleton global mouse click listener using XInput2 on the X11 root window.
// Automatically starts when the first callback is registered and stops when
// all callbacks are removed.
//
// Usage:
//   auto& listener = X11GlobalClickListener::instance();
//   auto id = listener.add_listener([](const ClickEvent& e) { ... });
//   // ... later ...
//   listener.remove_listener(id);
//
class X11GlobalClickListener {
public:
  enum class Button { Left, Right };

  struct ClickEvent {
    int x;
    int y;
    Button button;
  };

  using ClickHandler = std::function<void(const ClickEvent&)>;
  using ListenerId = uint64_t;

  static X11GlobalClickListener& instance();

  // Register a click handler. Starts the listener thread if not already running.
  // Returns an ID used to remove the handler later.
  ListenerId add_listener(ClickHandler handler);

  // Remove a previously registered handler. Stops the listener thread
  // if no handlers remain.
  void remove_listener(ListenerId id);

  ~X11GlobalClickListener();

  X11GlobalClickListener(const X11GlobalClickListener&) = delete;
  X11GlobalClickListener& operator=(const X11GlobalClickListener&) = delete;

private:
  X11GlobalClickListener() = default;

  void start_locked();  // must hold mutex_
  void stop_locked();   // must hold mutex_

  std::mutex mutex_;
  std::unordered_map<ListenerId, ClickHandler> handlers_;
  ListenerId next_id_ = 1;

  std::atomic<bool> listening_{false};
  std::unique_ptr<std::thread> thread_;
  int pipe_fd_[2] = {-1, -1};
};

} // namespace viewshell
