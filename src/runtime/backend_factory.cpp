#include "runtime/backend_factory.h"

#include <memory>

#include "runtime/backend_runtime.h"

#ifdef __APPLE__
#include "runtime/macos_backend_runtime.h"
#elif defined(_WIN32)
#include "runtime/win32_backend_runtime.h"
#else
#include "runtime/x11_backend_runtime.h"
#endif

namespace viewshell {

std::unique_ptr<BackendRuntime> BackendFactory::create() {
#ifdef __APPLE__
  return std::make_unique<MacOSBackendRuntime>();
#elif defined(_WIN32)
  return std::make_unique<Win32BackendRuntime>();
#else
  return std::make_unique<X11BackendRuntime>();
#endif
}

} // namespace viewshell
