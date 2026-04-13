#include "webview/resource_protocol.h"

#include <fstream>
#include <unordered_map>
#include <algorithm>

namespace viewshell {

namespace {

std::string mime_type_for_extension(const std::filesystem::path& path) {
  static const std::unordered_map<std::string, std::string> mime_map = {
      {".html", "text/html"},   {".htm", "text/html"},
      {".js", "application/javascript"}, {".mjs", "application/javascript"},
      {".css", "text/css"},     {".json", "application/json"},
      {".png", "image/png"},    {".jpg", "image/jpeg"},
      {".jpeg", "image/jpeg"},  {".gif", "image/gif"},
      {".svg", "image/svg+xml"},{".ico", "image/x-icon"},
      {".wasm", "application/wasm"},
      {".woff", "font/woff"},   {".woff2", "font/woff2"},
      {".ttf", "font/ttf"},     {".otf", "font/otf"},
      {".txt", "text/plain"},   {".xml", "application/xml"},
      {".webp", "image/webp"},  {".webm", "video/webm"},
      {".mp4", "video/mp4"},    {".mp3", "audio/mpeg"},
      {".ogg", "audio/ogg"},    {".wav", "audio/wav"},
  };
  auto ext = path.extension().string();
  std::string lower;
  lower.resize(ext.size());
  std::transform(ext.begin(), ext.end(), lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  auto it = mime_map.find(lower);
  if (it != mime_map.end()) return it->second;
  return "application/octet-stream";
}

std::string read_file_contents(const std::filesystem::path& path) {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) return {};
  return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

} // namespace

ResourceProtocol::ResourceProtocol(std::filesystem::path asset_root)
    : asset_root_(std::move(asset_root)) {}

const std::filesystem::path& ResourceProtocol::asset_root() const {
  return asset_root_;
}

std::string ResourceProtocol::entry_url() const {
  return "viewshell://app/" + entry_relative_.generic_string();
}

Result<ResourceProtocol> ResourceProtocol::from_entry_file(
    std::string_view entry_file, const WindowOptions& options) {
  std::filesystem::path entry(entry_file);

  std::error_code ec;
  auto normalized_entry = std::filesystem::weakly_canonical(entry);

  std::filesystem::path root;
  if (options.asset_root.has_value()) {
    auto normalized_root = std::filesystem::weakly_canonical(*options.asset_root);
    auto rel = std::filesystem::relative(normalized_entry, normalized_root, ec);
    if (ec || rel.empty() || (!rel.empty() && *rel.begin() == "..")) {
      return tl::unexpected(Error{
          .code = "resource_out_of_scope",
          .message = "Entry file is outside the specified asset root"});
    }

    auto canonical_entry = std::filesystem::canonical(entry, ec);
    if (ec) {
      return tl::unexpected(Error{
          .code = "resource_not_found",
          .message = "Entry file does not exist: " + std::string(entry_file)});
    }

    root = std::filesystem::canonical(*options.asset_root, ec);
    if (ec) {
      return tl::unexpected(Error{
          .code = "resource_not_found",
          .message = "Asset root does not exist: " + *options.asset_root});
    }
    auto entry_relative = std::filesystem::relative(canonical_entry, root);
    ResourceProtocol proto(root);
    proto.entry_relative_ = std::move(entry_relative);
    return proto;
  }

  auto canonical_entry = std::filesystem::canonical(entry, ec);
  if (ec) {
    return tl::unexpected(Error{
        .code = "resource_not_found",
        .message = "Entry file does not exist: " + std::string(entry_file)});
  }

  root = canonical_entry.parent_path();
  auto candidate = root.parent_path();
  while (!candidate.empty() && candidate != candidate.parent_path()) {
    if (std::filesystem::exists(candidate / "index.html")) {
      root = candidate;
      candidate = root.parent_path();
    } else {
      break;
    }
  }

  auto entry_relative = std::filesystem::relative(canonical_entry, root);
  ResourceProtocol proto(root);
  proto.entry_relative_ = std::move(entry_relative);
  return proto;
}

Result<ResourceResponse> ResourceProtocol::resolve(std::string_view url) const {
  std::string url_str(url);

  constexpr std::string_view scheme = "viewshell://";
  if (url_str.size() < scheme.size() ||
      url_str.substr(0, scheme.size()) != scheme) {
    return tl::unexpected(Error{
        .code = "resource_out_of_scope",
        .message = "Invalid URL scheme"});
  }

  auto authority = url_str.substr(scheme.size());
  auto slash_pos = authority.find('/');
  std::string host = (slash_pos == std::string::npos)
                         ? authority
                         : authority.substr(0, slash_pos);
  if (host != "app") {
    return tl::unexpected(Error{
        .code = "resource_out_of_scope",
        .message = "Invalid host: must be 'app'"});
  }

  std::string path_str = (slash_pos == std::string::npos)
                              ? "/"
                              : authority.substr(slash_pos);
  if (path_str.empty() || path_str[0] != '/') {
    path_str = "/" + path_str;
  }

  std::filesystem::path request_path(path_str.substr(1));

  std::error_code ec;
  auto full_path = asset_root_ / request_path;
  auto normalized = std::filesystem::weakly_canonical(full_path, ec);
  if (ec) {
    normalized = full_path;
  }

  auto normalized_root = std::filesystem::weakly_canonical(asset_root_);
  auto rel = std::filesystem::relative(normalized, normalized_root, ec);
  if (ec || (!rel.empty() && *rel.begin() == "..")) {
    return tl::unexpected(Error{
        .code = "resource_out_of_scope",
        .message = "Path escapes asset root"});
  }

  auto canonical_request = std::filesystem::canonical(full_path, ec);
  if (ec) {
    return tl::unexpected(Error{
        .code = "resource_not_found",
        .message = "Resource not found: " + std::string(request_path)});
  }

  auto body = read_file_contents(canonical_request);
  if (body.empty() && std::filesystem::file_size(canonical_request) > 0) {
    return tl::unexpected(Error{
        .code = "resource_not_found",
        .message = "Failed to read resource: " + std::string(request_path)});
  }

  return ResourceResponse{
      .mime_type = mime_type_for_extension(canonical_request),
      .body = std::move(body)};
}

} // namespace viewshell
