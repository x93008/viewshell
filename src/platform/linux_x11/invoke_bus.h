#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <viewshell/types.h>

namespace viewshell {

struct NativeEvent {
  std::string name;
  Json payload;
};

class EventSubscription {
public:
  EventSubscription(std::function<void()> unsubscriber);
  bool off();

private:
  std::function<void()> unsubscriber_;
  bool active_ = true;
};

class InvokeBus {
public:
  Result<Json> dispatch(const std::string& command, const Json& args);

  std::shared_ptr<EventSubscription> subscribe(
      const std::string& event_name,
      std::function<void(const Json&)> callback);

  bool emit(const std::string& event_name, const Json& payload);

  Result<void> register_command(const std::string& name, CommandHandler handler);
  void drop_subscriptions(const std::string& reason);

private:
  friend bool ForceSubscriptionDropForTest(InvokeBus&, std::string_view);
  std::unordered_map<std::string, CommandHandler> commands_;
  std::unordered_map<std::string, std::vector<std::shared_ptr<std::function<void(const Json&)>>>> subscribers_;
};

} // namespace viewshell
