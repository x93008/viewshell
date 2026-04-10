#include <viewshell/bridge_handle.h>
#include "runtime_state.h"

namespace viewshell {

BridgeHandle::BridgeHandle(std::shared_ptr<RuntimeWindowState> state)
    : state_(std::move(state)) {}

Result<void> BridgeHandle::register_command(std::string name, CommandHandler handler) {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", ""});
  }
  if (state_->command_registry.count(name)) {
    return tl::unexpected(Error{"command_already_registered",
        "command '" + name + "' is already registered"});
  }
  state_->command_registry.emplace(std::move(name), std::move(handler));
  return {};
}

Result<void> BridgeHandle::emit(std::string, const Json&) {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", ""});
  }
  return tl::unexpected(Error{"bridge_unavailable", "bridge is not active"});
}

} // namespace viewshell
