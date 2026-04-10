#pragma once

#include <viewshell/types.h>
#include <viewshell/options.h>

namespace viewshell {

namespace detail {
Result<NormalizedAppOptions> normalize_app_options_for_test(const AppOptions& options);
}

class Application {
public:
  static Result<Application> create(const AppOptions& options);

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) noexcept;
  Application& operator=(Application&&) noexcept;
  ~Application();

private:
  explicit Application(NormalizedAppOptions opts);

  NormalizedAppOptions opts_;
};

} // namespace viewshell
