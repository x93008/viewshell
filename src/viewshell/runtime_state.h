#pragma once

#include <string>
#include <unordered_map>
#include <viewshell/types.h>

namespace viewshell {

struct RuntimeWindowState {
  bool has_window = false;
  bool is_closed = false;
  std::unordered_map<std::string, CommandHandler> command_registry;
};

} // namespace viewshell
