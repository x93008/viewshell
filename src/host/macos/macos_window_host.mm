#ifdef __APPLE__

#include "host/macos/macos_window_host.h"
#include "host/window_api_bootstrap.h"

#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include <nlohmann/json.hpp>
#include <string>

#include "bridge/invoke_bus.h"
#include "viewshell/runtime_state.h"

static std::string unsupported_webview_message() {
  return "macOS webview backend not implemented yet";
}

static NSString* to_nsstring(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
}

namespace viewshell { class MacOSWindowHost; }

@interface ViewshellWindowDelegate : NSObject<NSWindowDelegate>
@property(nonatomic, assign) viewshell::MacOSWindowHost* host;
@end

@interface ViewshellWebMessageHandler : NSObject<WKScriptMessageHandler>
@property(nonatomic, assign) viewshell::MacOSWindowHost* host;
@end

@interface ViewshellNavigationDelegate : NSObject<WKNavigationDelegate>
@property(nonatomic, assign) viewshell::MacOSWindowHost* host;
@end

@interface ViewshellBorderlessWindow : NSWindow
@end

@implementation ViewshellBorderlessWindow
- (BOOL)canBecomeKeyWindow { return YES; }
- (BOOL)canBecomeMainWindow { return YES; }
@end

@implementation ViewshellWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
  if (self.host) {
    (void)self.host->close();
  }
  return NO;
}

@end

@implementation ViewshellWebMessageHandler

- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message {
  if (!self.host || ![message.body isKindOfClass:[NSString class]]) {
    return;
  }
  std::string body([(NSString*)message.body UTF8String]);
  self.host->handle_script_message(body);
}

@end

@implementation ViewshellNavigationDelegate

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {
  if (!self.host) {
    decisionHandler(WKNavigationActionPolicyAllow);
    return;
  }

  NSURL* url = navigationAction.request.URL;
  NSString* absolute = url ? url.absoluteString : @"";
  std::string request_url([absolute UTF8String]);
  decisionHandler(self.host->should_allow_navigation(request_url) ? WKNavigationActionPolicyAllow : WKNavigationActionPolicyCancel);
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation {
  if (!self.host) {
    return;
  }
  NSString* url = webView.URL ? webView.URL.absoluteString : @"";
  self.host->notify_page_load(std::string([url UTF8String]), "started", std::nullopt);
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
  if (!self.host) {
    return;
  }
  NSString* url = webView.URL ? webView.URL.absoluteString : @"";
  self.host->notify_page_load(std::string([url UTF8String]), "finished", std::nullopt);
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error {
  if (!self.host) {
    return;
  }
  NSString* url = webView.URL ? webView.URL.absoluteString : @"";
  self.host->notify_page_load(
      std::string([url UTF8String]),
      "finished",
      error ? std::optional<std::string>(std::to_string((long long)error.code)) : std::optional<std::string>());
}

@end

namespace viewshell {

namespace {

constexpr const char* kMacBridgeBootstrap = R"JS((function () {
  if (!window.webkit || !window.webkit.messageHandlers || !window.webkit.messageHandlers.viewshellBridge || window.__viewshell) return;

  var nextRequestId = 1;
  var pending = new Map();
  var listeners = new Map();

  function post(kind, name, payload, requestId) {
    window.webkit.messageHandlers.viewshellBridge.postMessage(JSON.stringify({
      kind: kind,
      name: name || null,
      payload: payload || {},
      requestId: requestId || null
    }));
  }

  window.addEventListener('viewshell:message', function (event) {
    var detail = event.detail || {};
    if (detail.kind === 'invoke_result' && detail.requestId != null) {
      var entry = pending.get(detail.requestId);
      if (!entry) return;
      pending.delete(detail.requestId);
      if (detail.ok) {
        entry.resolve(detail.payload || {});
      } else {
        entry.reject(detail.error || { code: 'bridge_error', message: 'unknown bridge error' });
      }
      return;
    }
    if (detail.kind === 'native_event') {
      var entries = listeners.get(detail.name) || [];
      entries.slice().forEach(function (listener) {
        listener(detail.payload || {});
      });
    }
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
    },
    on: function (name, listener) {
      var entries = listeners.get(name) || [];
      var wasEmpty = entries.length === 0;
      entries.push(listener);
      listeners.set(name, entries);
      if (wasEmpty) post('subscribe', name, {}, null);
      return function () { window.__viewshell.off(name, listener); };
    },
    off: function (name, listener) {
      var entries = listeners.get(name) || [];
      var next = entries.filter(function (entry) { return entry !== listener; });
      listeners.set(name, next);
      if (entries.length > 0 && next.length === 0) post('unsubscribe', name, {}, null);
    }
  };

  window.webkit.messageHandlers.viewshellDrag = {
    postMessage: function (payload) {
      post('drag', null, { value: payload || 'drag' }, null);
    }
  };

  window.__viewshellWinClose = function () {
    post('close', null, {}, null);
  };
})();)JS";

bool all_windows_closed(const std::shared_ptr<RuntimeAppState>& app_state) {
  for (const auto& window : app_state->windows) {
    if (window && !window->is_closed) {
      return false;
    }
  }
  return true;
}

} // namespace

MacOSWindowHost::MacOSWindowHost(std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state)
    : app_state_(std::move(app_state)),
      window_state_(std::move(window_state)) {}

MacOSWindowHost::~MacOSWindowHost() {
  WKWebView* webview = (WKWebView*)webview_;
  if (webview) {
    [webview removeFromSuperview];
    [webview release];
  }
  NSWindow* window = (NSWindow*)window_;
  if (window) {
    [window close];
    [window release];
  }
  ViewshellWindowDelegate* delegate = (ViewshellWindowDelegate*)delegate_;
  if (delegate) {
    [delegate release];
  }
  ViewshellWebMessageHandler* message_handler = (ViewshellWebMessageHandler*)message_handler_;
  if (message_handler) {
    [message_handler release];
  }
  ViewshellNavigationDelegate* navigation_delegate = (ViewshellNavigationDelegate*)navigation_delegate_;
  if (navigation_delegate) {
    [navigation_delegate release];
  }
  WKUserContentController* user_content = (WKUserContentController*)user_content_controller_;
  if (user_content) {
    [user_content release];
  }
}

Result<std::shared_ptr<MacOSWindowHost>> MacOSWindowHost::create(
    std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state,
    const WindowOptions& options) {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  auto host = std::shared_ptr<MacOSWindowHost>(
      new MacOSWindowHost(std::move(app_state), std::move(window_state)));
  host->apply_common_options(options);
  host->invoke_bus_ = std::make_unique<InvokeBus>();

  NSRect rect = NSMakeRect(options.x.value_or(100), options.y.value_or(100), options.width, options.height);
  NSWindowStyleMask style = host->borderless_ ? NSWindowStyleMaskBorderless : (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
  NSWindow* window = host->borderless_
    ? [[ViewshellBorderlessWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO]
    : [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
  if (!window) {
    return tl::unexpected(Error{"window_creation_failed", "failed to create NSWindow"});
  }

  ViewshellWindowDelegate* delegate = [[ViewshellWindowDelegate alloc] init];
  delegate.host = host.get();
  [window setDelegate:delegate];
  [window makeKeyAndOrderFront:nil];
  if (host->always_on_top_) {
    [window setLevel:NSFloatingWindowLevel];
  }

  if (!host->show_in_taskbar_) {
    [window setCollectionBehavior:([window collectionBehavior] | NSWindowCollectionBehaviorTransient)];
  }

  WKWebViewConfiguration* configuration = [[WKWebViewConfiguration alloc] init];
  [[configuration preferences] setValue:@YES forKey:@"developerExtrasEnabled"];
  WKUserContentController* user_content = [[WKUserContentController alloc] init];
  ViewshellWebMessageHandler* message_handler = [[ViewshellWebMessageHandler alloc] init];
  ViewshellNavigationDelegate* navigation_delegate = [[ViewshellNavigationDelegate alloc] init];
  message_handler.host = host.get();
  navigation_delegate.host = host.get();
  [user_content addScriptMessageHandler:message_handler name:@"viewshellBridge"];
  WKUserScript* bootstrap = [[WKUserScript alloc] initWithSource:to_nsstring(kMacBridgeBootstrap) injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:NO];
  [user_content addUserScript:bootstrap];
  [configuration setUserContentController:user_content];
  WKWebView* webview = [[WKWebView alloc] initWithFrame:[[window contentView] bounds] configuration:configuration];
  [webview setNavigationDelegate:navigation_delegate];
  [webview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  if (host->borderless_) {
    [window setOpaque:NO];
    [window setBackgroundColor:[NSColor clearColor]];
    [webview setValue:@NO forKey:@"drawsBackground"];
    [window setAcceptsMouseMovedEvents:YES];
  }
  [[window contentView] addSubview:webview];
  [bootstrap release];
  [user_content release];
  [configuration release];

  host->window_ = (void*)[window retain];
  host->delegate_ = (void*)[delegate retain];
  host->webview_ = (void*)[webview retain];
  host->message_handler_ = (void*)[message_handler retain];
  host->navigation_delegate_ = (void*)[navigation_delegate retain];
  host->user_content_controller_ = (void*)[user_content retain];

  if (host->inject_window_api_) {
    (void)host->add_init_script(kWindowApiBootstrap);
  }

  if (options.asset_root.has_value() && !options.asset_root->empty()) {
    auto load_result = host->load_file(*options.asset_root);
    if (!load_result) {
      return tl::unexpected(load_result.error());
    }
  }

  return host;
}

void MacOSWindowHost::run_message_loop() {
  [NSApp activateIgnoringOtherApps:YES];
  [NSApp run];
}

Result<void> MacOSWindowHost::ensure_window() const {
  if (!window_) {
    return tl::unexpected(Error{"invalid_state", "window is not available"});
  }
  return {};
}

void MacOSWindowHost::update_style() {
  if (!window_) return;
  NSWindow* window = (NSWindow*)window_;
  NSWindowStyleMask style = borderless_ ? NSWindowStyleMaskBorderless : (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
  [window setStyleMask:style];
  [window setLevel:always_on_top_ ? NSFloatingWindowLevel : NSNormalWindowLevel];
}

Result<void> MacOSWindowHost::set_title(std::string_view title) {
  if (auto result = ensure_window(); !result) return result;
  NSWindow* window = (NSWindow*)window_;
  [window setTitle:to_nsstring(title)];
  return {};
}

Result<void> MacOSWindowHost::maximize() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ zoom:nil]; return {}; }
Result<void> MacOSWindowHost::unmaximize() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ zoom:nil]; return {}; }
Result<void> MacOSWindowHost::minimize() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ miniaturize:nil]; return {}; }
Result<void> MacOSWindowHost::unminimize() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ deminiaturize:nil]; return {}; }
Result<void> MacOSWindowHost::show() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ orderFront:nil]; return {}; }
Result<void> MacOSWindowHost::hide() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ orderOut:nil]; return {}; }
Result<void> MacOSWindowHost::focus() { if (auto r=ensure_window(); !r) return r; [(NSWindow*)window_ makeKeyAndOrderFront:nil]; [NSApp activateIgnoringOtherApps:YES]; return {}; }

Result<void> MacOSWindowHost::set_geometry(Geometry geometry) {
  if (auto r = ensure_window(); !r) return r;
  NSRect frame = NSMakeRect(geometry.x, geometry.y, geometry.width, geometry.height);
  [(NSWindow*)window_ setFrame:frame display:YES];
  return {};
}

Result<Geometry> MacOSWindowHost::get_geometry() const {
  if (auto r = ensure_window(); !r) return tl::unexpected(r.error());
  NSRect frame = [(NSWindow*)window_ frame];
  return Geometry{static_cast<int>(frame.origin.x), static_cast<int>(frame.origin.y), static_cast<int>(frame.size.width), static_cast<int>(frame.size.height)};
}

Result<void> MacOSWindowHost::set_size(Size size) {
  if (auto r = ensure_window(); !r) return r;
  NSWindow* window = (NSWindow*)window_;
  NSRect frame = [window frame];
  frame.size = NSMakeSize(size.width, size.height);
  [window setFrame:frame display:YES];
  return {};
}

Result<Size> MacOSWindowHost::get_size() const {
  if (auto r = ensure_window(); !r) return tl::unexpected(r.error());
  NSRect frame = [(NSWindow*)window_ frame];
  return Size{static_cast<int>(frame.size.width), static_cast<int>(frame.size.height)};
}

Result<void> MacOSWindowHost::set_position(Position pos) {
  if (auto r = ensure_window(); !r) return r;
  NSWindow* window = (NSWindow*)window_;
  NSRect frame = [window frame];
  frame.origin = NSMakePoint(pos.x, pos.y);
  [window setFrameOrigin:frame.origin];
  return {};
}

Result<Position> MacOSWindowHost::get_position() const {
  if (auto r = ensure_window(); !r) return tl::unexpected(r.error());
  NSRect frame = [(NSWindow*)window_ frame];
  return Position{static_cast<int>(frame.origin.x), static_cast<int>(frame.origin.y)};
}

Result<void> MacOSWindowHost::set_borderless(bool enabled) { borderless_ = enabled; update_style(); return {}; }
Result<void> MacOSWindowHost::set_always_on_top(bool enabled) { always_on_top_ = enabled; update_style(); return {}; }

Result<void> MacOSWindowHost::close() {
  if (auto r = ensure_window(); !r) return r;
  if (auto app = app_state_.lock()) {
    if (auto state = window_state_.lock()) {
      state->is_closed = true;
    }
    if (all_windows_closed(app)) {
      app->shutdown_started = true;
      app->run_exit_code = 0;
      [NSApp stop:nil];
    }
  }
  [(NSWindow*)window_ orderOut:nil];
  return {};
}

Result<void> MacOSWindowHost::load_url(std::string_view url) {
  if (auto r = ensure_window(); !r) return r;
  if (!webview_) return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()});
  NSURL* nsurl = [NSURL URLWithString:to_nsstring(url)];
  if (!nsurl) {
    return tl::unexpected(Error{"invalid_state", "invalid URL for WKWebView"});
  }
  NSURLRequest* request = [NSURLRequest requestWithURL:nsurl];
  [(WKWebView*)webview_ loadRequest:request];
  return {};
}

Result<void> MacOSWindowHost::load_file(std::string_view entry_file) {
  if (auto r = ensure_window(); !r) return r;
  if (!webview_) return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()});
  NSString* path = to_nsstring(entry_file);
  NSURL* file_url = [NSURL fileURLWithPath:path];
  if (!file_url) {
    return tl::unexpected(Error{"invalid_state", "invalid file path for WKWebView"});
  }
  NSURL* root_url = [file_url URLByDeletingLastPathComponent];
  [(WKWebView*)webview_ loadFileURL:file_url allowingReadAccessToURL:root_url];
  return {};
}

Result<void> MacOSWindowHost::reload() {
  if (auto r = ensure_window(); !r) return r;
  if (!webview_) return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()});
  [(WKWebView*)webview_ reload];
  return {};
}

Result<void> MacOSWindowHost::evaluate_script(std::string_view script) {
  if (auto r = ensure_window(); !r) return r;
  if (!webview_) return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()});
  [(WKWebView*)webview_ evaluateJavaScript:to_nsstring(script) completionHandler:nil];
  return {};
}

Result<void> MacOSWindowHost::add_init_script(std::string_view script) {
  init_scripts_.push_back(std::string(script));

  WKUserContentController* user_content = (WKUserContentController*)user_content_controller_;
  if (user_content) {
    WKUserScript* user_script = [[WKUserScript alloc] initWithSource:to_nsstring(script)
        injectionTime:WKUserScriptInjectionTimeAtDocumentStart
        forMainFrameOnly:NO];
    [user_content addUserScript:user_script];
    [user_script release];
  }

  if (webview_) {
    [(WKWebView*)webview_ evaluateJavaScript:to_nsstring(script) completionHandler:nil];
  }

  return {};
}
Result<void> MacOSWindowHost::open_devtools() {
  if (auto r = ensure_window(); !r) return r;
  if (!webview_) return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()});

  id webview = (WKWebView*)webview_;
  if (![webview respondsToSelector:NSSelectorFromString(@"_inspector")]) {
    return tl::unexpected(Error{"unsupported_by_backend", "WKWebView inspector API is unavailable"});
  }
  id inspector = [webview performSelector:NSSelectorFromString(@"_inspector")];
  if (inspector && [inspector respondsToSelector:NSSelectorFromString(@"show")]) {
    [inspector performSelector:NSSelectorFromString(@"show")];
    return {};
  }
  return tl::unexpected(Error{"unsupported_by_backend", "WKWebView inspector show API is unavailable"});
}

Result<void> MacOSWindowHost::close_devtools() {
  if (auto r = ensure_window(); !r) return r;
  if (!webview_) return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()});

  id webview = (WKWebView*)webview_;
  if (![webview respondsToSelector:NSSelectorFromString(@"_inspector")]) {
    return tl::unexpected(Error{"unsupported_by_backend", "WKWebView inspector API is unavailable"});
  }
  id inspector = [webview performSelector:NSSelectorFromString(@"_inspector")];
  if (inspector && [inspector respondsToSelector:NSSelectorFromString(@"close")]) {
    [inspector performSelector:NSSelectorFromString(@"close")];
    return {};
  }
  if (inspector && [inspector respondsToSelector:NSSelectorFromString(@"hide")]) {
    [inspector performSelector:NSSelectorFromString(@"hide")];
    return {};
  }
  return tl::unexpected(Error{"unsupported_by_backend", "WKWebView inspector close API is unavailable"});
}
Result<void> MacOSWindowHost::on_page_load(PageLoadHandler handler) {
  page_load_handlers_.push_back(std::move(handler));
  return {};
}
Result<void> MacOSWindowHost::set_navigation_handler(NavigationHandler handler) {
  navigation_handler_ = std::move(handler);
  return {};
}
Result<void> MacOSWindowHost::register_command(std::string name, CommandHandler handler) { return invoke_bus_->register_command(std::move(name), std::move(handler)); }

Result<void> MacOSWindowHost::emit(std::string name, const Json& payload) {
  if (!subscribed_events_.count(name) || !webview_) {
    return {};
  }
  Json message_out{{"kind", "native_event"}, {"name", name}, {"payload", payload}};
  dispatch_json_to_page(message_out);
  return {};
}

void MacOSWindowHost::dispatch_json_to_page(const Json& payload) {
  if (!webview_) {
    return;
  }
  NSString* js = [NSString stringWithFormat:@"window.dispatchEvent(new CustomEvent('viewshell:message', { detail: %@ }));", to_nsstring(payload.dump())];
  [(WKWebView*)webview_ evaluateJavaScript:js completionHandler:nil];
}

void MacOSWindowHost::begin_drag() {
  NSWindow* window = (NSWindow*)window_;
  NSEvent* event = [NSApp currentEvent];
  if (window && event) {
    [window performWindowDragWithEvent:event];
  }
}

void MacOSWindowHost::handle_script_message(std::string_view message) {
  auto parsed = nlohmann::json::parse(message, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) {
    return;
  }

  auto kind_it = parsed.find("kind");
  auto name_it = parsed.find("name");
  auto payload_it = parsed.find("payload");
  auto request_id_it = parsed.find("requestId");
  if (kind_it == parsed.end() || !kind_it->is_string()) {
    return;
  }

  std::string kind = *kind_it;
  std::string name = (name_it != parsed.end() && name_it->is_string()) ? std::string(*name_it) : std::string();
  Json payload = (payload_it != parsed.end()) ? *payload_it : Json::object();

  if (kind == "close") {
    (void)close();
    return;
  }
  if (kind == "drag") {
    begin_drag();
    return;
  }
  if (kind == "subscribe") {
    subscribed_events_.insert(name);
    return;
  }
  if (kind == "unsubscribe") {
    subscribed_events_.erase(name);
    return;
  }
  if (kind == "invoke") {
    // Handle built-in __wnd.* commands
    if (name.rfind("__wnd.", 0) == 0) {
      Json result_payload;
      Result<void> ok;
      if (handle_wnd_command(name, payload, result_payload, ok)) {
        Json message_out{{"kind", "invoke_result"}, {"name", name}, {"ok", static_cast<bool>(ok)}, {"payload", ok ? result_payload : Json::object()}};
        if (request_id_it != parsed.end() && request_id_it->is_number_unsigned()) {
          message_out["requestId"] = *request_id_it;
        }
        if (!ok) {
          message_out["error"] = Json{{"code", ok.error().code}, {"message", ok.error().message}};
        }
        dispatch_json_to_page(message_out);
        return;
      }
    }
    auto result = invoke_bus_->dispatch(name, payload);
    Json message_out{{"kind", "invoke_result"}, {"name", name}, {"ok", static_cast<bool>(result)}, {"payload", result ? *result : Json::object()}};
    if (request_id_it != parsed.end() && request_id_it->is_number_unsigned()) {
      message_out["requestId"] = *request_id_it;
    }
    if (!result) {
      message_out["error"] = Json{{"code", result.error().code}, {"message", result.error().message}};
    }
    dispatch_json_to_page(message_out);
    return;
  }
  if (kind == "emit" && subscribed_events_.count(name)) {
    Json message_out{{"kind", "native_event"}, {"name", name}, {"payload", payload}};
    dispatch_json_to_page(message_out);
  }
}

void MacOSWindowHost::notify_page_load(std::string url, std::string stage, std::optional<std::string> error_code) {
  PageLoadEvent event{std::move(url), std::move(stage), std::move(error_code)};
  for (auto& handler : page_load_handlers_) {
    handler(event);
  }
}

bool MacOSWindowHost::should_allow_navigation(std::string_view url) const {
  if (!navigation_handler_) {
    return true;
  }
  NavigationRequest request{std::string(url)};
  return navigation_handler_(request) == NavigationDecision::Allow;
}

Result<Capabilities> MacOSWindowHost::capabilities() const {
  Capabilities caps;
  caps.window.borderless = true;
  caps.window.transparent = true;
  caps.window.always_on_top = true;
  caps.window.native_drag = true;
  caps.webview.devtools = true;
  caps.webview.script_eval = true;
  caps.bridge.invoke = true;
  caps.bridge.native_events = true;
  return caps;
}

} // namespace viewshell

#endif
