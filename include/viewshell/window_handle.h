#pragma once

#include <string_view>
#include <viewshell/types.h>
#include <viewshell/options.h>
#include <viewshell/capabilities.h>
#include <viewshell/bridge_handle.h>

namespace viewshell {

class Application;

class WindowHandle {
public:
  Result<BridgeHandle> bridge();

  Result<void> set_title(std::string_view title);
  Result<void> maximize();
  Result<void> unmaximize();
  Result<void> minimize();
  Result<void> unminimize();
  Result<void> show();
  Result<void> hide();
  Result<void> focus();
  Result<void> set_geometry(Geometry geometry);
  Result<Geometry> get_geometry() const;
  Result<void> set_size(Size size);
  Result<Size> get_size() const;
  Result<void> set_position(Position position);
  Result<Position> get_position() const;
  Result<void> set_borderless(bool enabled);
  Result<void> set_always_on_top(bool enabled);
  Result<void> close();

  Result<void> load_url(std::string_view url);
  Result<void> load_file(std::string_view entry_file);
  Result<void> reload();
  Result<void> evaluate_script(std::string_view script);
  Result<void> add_init_script(std::string_view script);
  Result<void> open_devtools();
  Result<void> close_devtools();

  Result<void> on_page_load(PageLoadHandler handler);
  Result<void> set_navigation_handler(NavigationHandler handler);

  Result<Capabilities> capabilities() const;

private:
  friend class Application;
  friend void MarkWindowClosedForTest(WindowHandle&);
  friend void ArmCloseAcknowledgementForTest(WindowHandle&);
  explicit WindowHandle(std::shared_ptr<struct RuntimeWindowState> state)
      : state_(std::move(state)) {}
  std::shared_ptr<RuntimeWindowState> state_;
};

} // namespace viewshell
