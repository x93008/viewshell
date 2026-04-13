#include "linux_x11_window_host.h"

#include <memory>

#include "platform/linux_x11/bridge_driver.h"
#include "platform/linux_x11/invoke_bus.h"
#include "platform/linux_x11/webview_driver.h"
#include "platform/linux_x11/window_driver.h"
#include "viewshell/runtime_state.h"

#include <nlohmann/json.hpp>

namespace viewshell {

LinuxX11WindowHost::LinuxX11WindowHost(std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state)
    : app_state_(std::move(app_state)),
      window_state_(std::move(window_state)) {}

LinuxX11WindowHost::~LinuxX11WindowHost() = default;

Result<std::shared_ptr<LinuxX11WindowHost>> LinuxX11WindowHost::create(
    std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state,
    const WindowOptions& options) {
  auto host = std::shared_ptr<LinuxX11WindowHost>(
      new LinuxX11WindowHost(std::move(app_state), std::move(window_state)));
  auto weak_app_state = host->app_state_;
  auto weak_window_state = host->window_state_;

  host->window_driver_ = std::make_unique<WindowDriver>();
  host->webview_driver_ = std::make_unique<WebviewDriver>();
  host->bridge_driver_ = std::make_unique<BridgeDriver>();
  host->invoke_bus_ = std::make_unique<InvokeBus>();

  host->window_driver_->on_close = [app_state = weak_app_state,
                                       window_state = weak_window_state,
                                       driver = host->window_driver_.get()]() {
    if (auto app = app_state.lock()) {
      app->shutdown_started = true;
      app->run_exit_code = 0;
    }
    if (auto window = window_state.lock()) {
      window->is_closed = true;
    }
    driver->quit_main_loop();
  };

  auto native = host->window_driver_->create(options);
  if (!native) {
    return tl::unexpected(native.error());
  }

  auto attach = host->webview_driver_->attach(*native, options);
  if (!attach) {
    return tl::unexpected(attach.error());
  }

  auto bridge_attach = host->bridge_driver_->attach(*host->webview_driver_);
  if (!bridge_attach) {
    return tl::unexpected(bridge_attach.error());
  }

  host->bridge_driver_->on_raw_message = [invoke_bus = host->invoke_bus_.get(),
                                             bridge = host->bridge_driver_.get(),
                                             host_ptr = host.get()](std::string_view raw) {
    auto parsed = nlohmann::json::parse(raw, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
      return;
    }

    auto kind_it = parsed.find("kind");
    auto name_it = parsed.find("name");
    auto payload_it = parsed.find("payload");
    auto request_id_it = parsed.find("requestId");
    if (kind_it == parsed.end() || name_it == parsed.end() || !kind_it->is_string() || !name_it->is_string()) {
      return;
    }

    Json payload = payload_it != parsed.end() ? *payload_it : Json::object();
    std::string kind = *kind_it;
    std::string name = *name_it;

    if (kind == "invoke") {
      auto result = invoke_bus->dispatch(name, payload);
      Json message{{"kind", "invoke_result"}, {"name", name},
          {"ok", static_cast<bool>(result)},
          {"payload", result ? *result : Json::object()}};
      if (request_id_it != parsed.end() && request_id_it->is_number_unsigned()) {
        message["requestId"] = *request_id_it;
      }
      if (!result) {
        message["error"] = Json{{"code", result.error().code}, {"message", result.error().message}};
      }
      (void)bridge->post_to_page(message.dump());
      return;
    }

    if (kind == "subscribe") {
      host_ptr->subscribed_events_.insert(name);
      return;
    }

    if (kind == "unsubscribe") {
      host_ptr->subscribed_events_.erase(name);
      return;
    }

    if (kind == "emit") {
      if (host_ptr->subscribed_events_.count(name)) {
        Json message{{"kind", "native_event"}, {"name", name}, {"payload", payload}};
        (void)bridge->post_to_page(message.dump());
      }
    }
  };

  if (options.asset_root.has_value() && !options.asset_root->empty()) {
    auto load_result = host->webview_driver_->load_file(*options.asset_root);
    if (!load_result) {
      return tl::unexpected(load_result.error());
    }
  }

  auto show_result = host->window_driver_->show();
  if (!show_result) {
    return tl::unexpected(show_result.error());
  }

  return host;
}

void LinuxX11WindowHost::run_main_loop() {
  if (window_driver_) {
    window_driver_->run_main_loop();
  }
}

Result<void> LinuxX11WindowHost::set_title(std::string_view title) {
  return window_driver_->set_title(title);
}

Result<void> LinuxX11WindowHost::maximize() {
  return window_driver_->maximize();
}

Result<void> LinuxX11WindowHost::unmaximize() {
  return window_driver_->unmaximize();
}

Result<void> LinuxX11WindowHost::minimize() {
  return window_driver_->minimize();
}

Result<void> LinuxX11WindowHost::unminimize() {
  return window_driver_->unminimize();
}

Result<void> LinuxX11WindowHost::show() {
  return window_driver_->show();
}

Result<void> LinuxX11WindowHost::hide() {
  return window_driver_->hide();
}

Result<void> LinuxX11WindowHost::focus() {
  return window_driver_->focus();
}

Result<void> LinuxX11WindowHost::set_size(Size size) {
  return window_driver_->set_size(size);
}

Result<Size> LinuxX11WindowHost::get_size() const {
  return window_driver_->get_size();
}

Result<void> LinuxX11WindowHost::set_position(Position pos) {
  return window_driver_->set_position(pos);
}

Result<Position> LinuxX11WindowHost::get_position() const {
  return window_driver_->get_position();
}

Result<void> LinuxX11WindowHost::set_borderless(bool enabled) {
  return window_driver_->set_borderless(enabled);
}

Result<void> LinuxX11WindowHost::set_always_on_top(bool enabled) {
  return window_driver_->set_always_on_top(enabled);
}

Result<void> LinuxX11WindowHost::close() {
  if (auto window_state = window_state_.lock()) {
    window_state->is_closed = true;
  }
  return window_driver_->close();
}

Result<void> LinuxX11WindowHost::load_url(std::string_view url) {
  return webview_driver_->load_url(url);
}

Result<void> LinuxX11WindowHost::load_file(std::string_view entry_file) {
  return webview_driver_->load_file(entry_file);
}

Result<void> LinuxX11WindowHost::reload() {
  return webview_driver_->reload();
}

Result<void> LinuxX11WindowHost::evaluate_script(std::string_view script) {
  return webview_driver_->evaluate_script(script);
}

Result<void> LinuxX11WindowHost::add_init_script(std::string_view script) {
  return webview_driver_->add_init_script(script);
}

Result<void> LinuxX11WindowHost::open_devtools() {
  return webview_driver_->open_devtools();
}

Result<void> LinuxX11WindowHost::close_devtools() {
  return webview_driver_->close_devtools();
}

Result<void> LinuxX11WindowHost::on_page_load(PageLoadHandler handler) {
  return webview_driver_->on_page_load(std::move(handler));
}

Result<void> LinuxX11WindowHost::set_navigation_handler(NavigationHandler handler) {
  return webview_driver_->set_navigation_handler(std::move(handler));
}

Result<Capabilities> LinuxX11WindowHost::capabilities() const {
  if (auto window_state = window_state_.lock()) {
    if (window_state->resolved_capabilities) {
      return *window_state->resolved_capabilities;
    }
  }
  return webview_driver_->capabilities();
}

Result<void> LinuxX11WindowHost::register_command(std::string name, CommandHandler handler) {
  return invoke_bus_->register_command(std::move(name), std::move(handler));
}

Result<void> LinuxX11WindowHost::emit(std::string name, const Json& payload) {
  if (!bridge_driver_->is_ready()) {
    return tl::unexpected(Error{"bridge_unavailable", "bridge is not active"});
  }
  if (!subscribed_events_.count(name)) {
    return {};
  }
  Json message{{"kind", "native_event"}, {"name", name}, {"payload", payload}};
  auto raw = message.dump();
  auto result = bridge_driver_->post_to_page(raw);
  if (!result) {
    return tl::unexpected(result.error());
  }
  return {};
}

} // namespace viewshell
