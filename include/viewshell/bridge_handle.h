#pragma once

#include <string>
#include <viewshell/types.h>

namespace viewshell {

class BridgeHandle {
public:
  Result<void> register_command(std::string name, CommandHandler handler);
  Result<void> emit(std::string name, const Json& payload);

private:
  friend class WindowHandle;
  explicit BridgeHandle(std::shared_ptr<struct RuntimeWindowState> state);
  std::shared_ptr<RuntimeWindowState> state_;
};

} // namespace viewshell
