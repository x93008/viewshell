#include "runtime/backend_factory.h"

#include <memory>

#include "runtime/backend_runtime.h"

#ifdef _WIN32
#include "platform/windows/windows_backend_runtime.h"
#else
#include "platform/linux_x11/linux_x11_backend_runtime.h"
#endif

namespace viewshell {

std::unique_ptr<BackendRuntime> BackendFactory::create() {
#ifdef _WIN32
  return std::make_unique<WindowsBackendRuntime>();
#else
  return std::make_unique<LinuxX11BackendRuntime>();
#endif
}

} // namespace viewshell
