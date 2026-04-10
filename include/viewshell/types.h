#pragma once

#include <string>
#include <optional>
#include <functional>
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

namespace viewshell {

using Json = nlohmann::json;

struct Error {
  std::string code;
  std::string message;
  Json details;
};

template <typename T>
using Result = tl::expected<T, Error>;

struct Size {
  int width = 0;
  int height = 0;
};

struct Position {
  int x = 0;
  int y = 0;
};

enum class NavigationDecision { Allow, Deny };

struct PageLoadEvent {
  std::string url;
  std::string stage;
  std::optional<std::string> error_code;
};

struct NavigationRequest {
  std::string url;
};

using PageLoadHandler = std::function<void(const PageLoadEvent&)>;
using NavigationHandler = std::function<NavigationDecision(const NavigationRequest&)>;
using CommandHandler = std::function<Result<Json>(const Json&)>;

} // namespace viewshell
