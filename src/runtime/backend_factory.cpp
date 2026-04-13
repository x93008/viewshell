#include "runtime/backend_factory.h"

#include <memory>

#include "runtime/backend_runtime.h"

#ifdef _WIN32
#include "runtime/win32_backend_runtime.h"
#else
#include "runtime/x11_backend_runtime.h"
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
