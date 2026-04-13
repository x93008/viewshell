#include "bridge/invoke_bus.h"
#include <algorithm>

namespace viewshell {

EventSubscription::EventSubscription(std::function<void()> unsubscriber)
    : unsubscriber_(std::move(unsubscriber)) {}

bool EventSubscription::off() {
  if (!active_) return true;
  active_ = false;
  if (unsubscriber_) unsubscriber_();
  return true;
}

Result<Json> InvokeBus::dispatch(const std::string& command, const Json& args) {
  auto it = commands_.find(command);
  if (it == commands_.end()) {
    return tl::unexpected(Error{"command_not_found", "unknown command: " + command});
  }
  return it->second(args);
}

std::shared_ptr<EventSubscription> InvokeBus::subscribe(
    const std::string& event_name,
    std::function<void(const Json&)> callback) {
  auto wrapper = std::make_shared<std::function<void(const Json&)>>(std::move(callback));
  subscribers_[event_name].push_back(wrapper);

  return std::make_shared<EventSubscription>([this, event_name, wrapper]() {
    auto& vec = subscribers_[event_name];
    auto* raw = wrapper.get();
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [raw](const auto& sp) { return sp.get() == raw; }), vec.end());
  });
}

bool InvokeBus::emit(const std::string& event_name, const Json& payload) {
  auto it = subscribers_.find(event_name);
  if (it == subscribers_.end() || it->second.empty()) return false;

  auto& vec = it->second;
  for (auto& cb : vec) {
    (*cb)(payload);
  }
  return true;
}

Result<void> InvokeBus::register_command(const std::string& name, CommandHandler handler) {
  if (commands_.count(name)) {
    return tl::unexpected(Error{"command_already_registered",
        "command '" + name + "' is already registered"});
  }
  commands_.emplace(name, std::move(handler));
  return {};
}

void InvokeBus::drop_subscriptions(const std::string& reason) {
  subscribers_.clear();
}

} // namespace viewshell
