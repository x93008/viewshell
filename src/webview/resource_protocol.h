#pragma once

#include <string>
#include <filesystem>
#include <viewshell/types.h>
#include <viewshell/options.h>

namespace viewshell {

struct ResourceResponse {
  std::string mime_type;
  std::string body;
};

class ResourceProtocol {
public:
  explicit ResourceProtocol(std::filesystem::path asset_root);

  static Result<ResourceProtocol> from_entry_file(
      std::string_view entry_file, const WindowOptions& options);

  Result<ResourceResponse> resolve(std::string_view url) const;

  const std::filesystem::path& asset_root() const;
  std::string entry_url() const;

private:
  std::filesystem::path asset_root_;
  std::filesystem::path entry_relative_;
};

} // namespace viewshell
