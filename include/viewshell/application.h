#pragma once

#include <string>
#include <tl/expected.hpp>

namespace viewshell {

struct Error {
  std::string code;
  std::string message;
};

template <typename T>
using Result = tl::expected<T, Error>;

struct AppOptions {
  int bridge_timeout_ms = 5000;
};

class Application {
public:
  static Result<Application> create(const AppOptions& options);

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) noexcept;
  Application& operator=(Application&&) noexcept;
  ~Application();

private:
  explicit Application(int bridge_timeout_ms);

  int bridge_timeout_ms_;
};

} // namespace viewshell
