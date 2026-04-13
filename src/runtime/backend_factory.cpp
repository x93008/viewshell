#include "runtime/backend_factory.h"

#include <memory>

#include "platform/linux_x11/linux_x11_backend_runtime.h"
#include "runtime/backend_runtime.h"

namespace viewshell {

std::unique_ptr<BackendRuntime> BackendFactory::create() {
  return std::make_unique<LinuxX11BackendRuntime>();
}

} // namespace viewshell
