#include "bridge_driver.h"
#include "webview_driver.h"

#include <string>

namespace viewshell {

namespace {

constexpr const char* kBridgeBootstrap = R"JS((function () {
  if (window.__viewshell) return;

  var nextRequestId = 1;
  var pending = new Map();

  function post(kind, name, payload, requestId) {
    if (!window.webkit || !window.webkit.messageHandlers || !window.webkit.messageHandlers.viewshellBridge) {
      return;
    }
    window.webkit.messageHandlers.viewshellBridge.postMessage(JSON.stringify({
      kind: kind,
      name: name,
      payload: payload || {},
      requestId: requestId || null
    }));
  }

  window.addEventListener('viewshell:message', function (event) {
    var detail = event.detail || {};
    if (detail.kind !== 'invoke_result' || detail.requestId == null) {
      return;
    }

    var entry = pending.get(detail.requestId);
    if (!entry) {
      return;
    }

    pending.delete(detail.requestId);
    if (detail.ok) {
      entry.resolve(detail.payload || {});
      return;
    }

    entry.reject(detail.error || { code: 'bridge_error', message: 'unknown bridge error' });
  });

  window.__viewshell = {
    invoke: function (name, args) {
      var requestId = nextRequestId++;
      return new Promise(function (resolve, reject) {
        pending.set(requestId, { resolve: resolve, reject: reject });
        post('invoke', name, args || {}, requestId);
      });
    },
    emit: function (name, payload) {
      post('emit', name, payload || {}, null);
    }
  };
})();)JS";

} // namespace

Result<void> BridgeDriver::attach(WebviewDriver& webview) {
  webview_ = &webview;
  auto script_result = webview_->add_init_script(kBridgeBootstrap);
  if (!script_result) {
    return tl::unexpected(script_result.error());
  }

  auto handler_result = webview_->register_script_message_handler(
      "viewshellBridge",
      [this](std::string_view payload) {
        bridge_ready_ = true;
        if (on_bridge_ready) {
          on_bridge_ready();
        }
        last_posted_ = std::string(payload);
        if (on_raw_message) {
          on_raw_message(payload);
        }
      });
  if (!handler_result) {
    return tl::unexpected(handler_result.error());
  }

  return {};
}

Result<void> BridgeDriver::post_to_page(std::string_view raw_message) {
  if (!bridge_ready_) {
    return tl::unexpected(Error{"invalid_state", "bridge not ready"});
  }
  last_posted_ = std::string(raw_message);
  if (webview_) {
    std::string script = "window.dispatchEvent(new CustomEvent('viewshell:message', { detail: "
        + std::string(raw_message) + " }));";
    auto result = webview_->evaluate_script(script);
    if (!result) {
      return tl::unexpected(result.error());
    }
  }
  return {};
}

} // namespace viewshell
