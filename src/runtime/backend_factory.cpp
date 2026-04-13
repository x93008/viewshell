#include "runtime/backend_factory.h"

#include <memory>

#include "runtime/backend_runtime.h"

#ifdef _WIN32
#include "platform/windows/win32_backend_runtime.h"
#else
#include "platform/x11/x11_backend_runtime.h"
#endif

namespace viewshell {

std::unique_ptr<BackendRuntime> BackendFactory::create() {
#ifdef _WIN32
  return std::make_unique<Win32BackendRuntime>();
#else
  return std::make_unique<X11BackendRuntime>();
#endif
}

} // namespace viewshell
