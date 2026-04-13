#include <viewshell/bridge_handle.h>
#include "runtime_state.h"
#include "runtime/window_host.h"

namespace viewshell {

BridgeHandle::BridgeHandle(std::shared_ptr<RuntimeWindowState> state)
    : state_(std::move(state)) {}

Result<void> BridgeHandle::register_command(std::string name, CommandHandler handler) {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", ""});
  }
  if (!state_->window_host) {
    return tl::unexpected(Error{"bridge_unavailable", "bridge is not active"});
  }
  return state_->window_host->register_command(std::move(name), std::move(handler));
}

Result<void> BridgeHandle::emit(std::string name, const Json& payload) {
  if (state_->is_closed) {
    return tl::unexpected(Error{"window_closed", ""});
  }
  if (!state_->window_host) {
    return tl::unexpected(Error{"bridge_unavailable", "bridge is not active"});
  }
  return state_->window_host->emit(std::move(name), payload);
}

} // namespace viewshell
