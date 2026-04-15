#include "example_asset_path.h"

#include <string>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#else
#include <filesystem>
#endif

namespace viewshell::examples {

#ifdef __APPLE__

namespace {

std::string join_paths(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (a.back() == '/') return a + b;
  return a + "/" + b;
}

std::string dirname_copy(const std::string& path) {
  auto pos = path.find_last_of('/');
  if (pos == std::string::npos) return ".";
  if (pos == 0) return "/";
  return path.substr(0, pos);
}

bool file_exists(const std::string& path) {
  return access(path.c_str(), F_OK) == 0;
}

std::string resolve_executable_dir(const char* argv0) {
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::string buffer(size, '\0');
  if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
    char real_path[PATH_MAX];
    if (realpath(buffer.c_str(), real_path)) {
      return dirname_copy(real_path);
    }
  }
  return argv0 ? dirname_copy(argv0) : ".";
}

} // namespace

std::string resolve_example_asset_path(const char* argv0, const char* entry_file) {
  std::string exe_dir = resolve_executable_dir(argv0);
  std::string candidate = join_paths(join_paths(exe_dir, "app"), entry_file);
  if (file_exists(candidate)) {
    return candidate;
  }
  return join_paths(join_paths(dirname_copy(exe_dir), "app"), entry_file);
}

#else

std::string resolve_example_asset_path(const char* argv0, const char* entry_file) {
  std::string exe_arg(argv0 ? argv0 : "");
  auto exe_dir = std::filesystem::canonical(std::filesystem::path(exe_arg)).parent_path();
  auto asset_path = exe_dir / "app" / entry_file;
  if (!std::filesystem::exists(asset_path)) {
    asset_path = exe_dir.parent_path() / "app" / entry_file;
  }
  return asset_path.string();
}

#endif

} // namespace viewshell::examples
