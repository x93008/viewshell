#ifdef __APPLE__

#include "host/macos/macos_window_host.h"

#import <AppKit/AppKit.h>

#include <string>

#include "viewshell/runtime_state.h"

namespace viewshell {

namespace {

std::string unsupported_webview_message() {
  return "macOS webview backend not implemented yet";
}

std::string to_utf8(NSString* value) {
  if (!value) return {};
  return std::string([value UTF8String]);
}

NSString* to_nsstring(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
}

class MacOSWindowHost;

@interface ViewshellWindowDelegate : NSObject<NSWindowDelegate>
@property(nonatomic, assign) viewshell::MacOSWindowHost* host;
@end

} // namespace

class MacOSWindowHost;

MacOSWindowHost::MacOSWindowHost(std::shared_ptr<RuntimeAppState> app_state,
    std::shared_ptr<RuntimeWindowState> window_state)
    : app_state_(std::move(app_state)),
      window_state_(std::move(window_state)) {}

@implementation ViewshellWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
  if (self.host) {
    (void)self.host->close();
  }
  return NO;
}

@end

MacOSWindowHost::~MacOSWindowHost() {
  NSWindow* window = (__bridge NSWindow*)window_;
  if (window) {
    [window close];
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
  host->borderless_ = options.borderless;
  host->always_on_top_ = options.always_on_top;

  NSRect rect = NSMakeRect(options.x.value_or(100), options.y.value_or(100), options.width, options.height);
  NSWindowStyleMask style = host->borderless_ ? NSWindowStyleMaskBorderless : (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
  NSWindow* window = [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
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

  host->window_ = (__bridge_retained void*)window;
  host->delegate_ = (__bridge_retained void*)delegate;
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
  NSWindow* window = (__bridge NSWindow*)window_;
  NSWindowStyleMask style = borderless_ ? NSWindowStyleMaskBorderless : (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
  [window setStyleMask:style];
  [window setLevel:always_on_top_ ? NSFloatingWindowLevel : NSNormalWindowLevel];
}

Result<void> MacOSWindowHost::set_title(std::string_view title) {
  if (auto result = ensure_window(); !result) return result;
  NSWindow* window = (__bridge NSWindow*)window_;
  [window setTitle:to_nsstring(title)];
  return {};
}

Result<void> MacOSWindowHost::maximize() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ zoom:nil]; return {}; }
Result<void> MacOSWindowHost::unmaximize() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ zoom:nil]; return {}; }
Result<void> MacOSWindowHost::minimize() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ miniaturize:nil]; return {}; }
Result<void> MacOSWindowHost::unminimize() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ deminiaturize:nil]; return {}; }
Result<void> MacOSWindowHost::show() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ orderFront:nil]; return {}; }
Result<void> MacOSWindowHost::hide() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ orderOut:nil]; return {}; }
Result<void> MacOSWindowHost::focus() { if (auto r=ensure_window(); !r) return r; [(__bridge NSWindow*)window_ makeKeyAndOrderFront:nil]; [NSApp activateIgnoringOtherApps:YES]; return {}; }

Result<void> MacOSWindowHost::set_size(Size size) {
  if (auto r = ensure_window(); !r) return r;
  NSWindow* window = (__bridge NSWindow*)window_;
  NSRect frame = [window frame];
  frame.size = NSMakeSize(size.width, size.height);
  [window setFrame:frame display:YES];
  return {};
}

Result<Size> MacOSWindowHost::get_size() const {
  if (auto r = ensure_window(); !r) return tl::unexpected(r.error());
  NSRect frame = [(__bridge NSWindow*)window_ frame];
  return Size{static_cast<int>(frame.size.width), static_cast<int>(frame.size.height)};
}

Result<void> MacOSWindowHost::set_position(Position pos) {
  if (auto r = ensure_window(); !r) return r;
  NSWindow* window = (__bridge NSWindow*)window_;
  NSRect frame = [window frame];
  frame.origin = NSMakePoint(pos.x, pos.y);
  [window setFrameOrigin:frame.origin];
  return {};
}

Result<Position> MacOSWindowHost::get_position() const {
  if (auto r = ensure_window(); !r) return tl::unexpected(r.error());
  NSRect frame = [(__bridge NSWindow*)window_ frame];
  return Position{static_cast<int>(frame.origin.x), static_cast<int>(frame.origin.y)};
}

Result<void> MacOSWindowHost::set_borderless(bool enabled) { borderless_ = enabled; update_style(); return {}; }
Result<void> MacOSWindowHost::set_always_on_top(bool enabled) { always_on_top_ = enabled; update_style(); return {}; }

Result<void> MacOSWindowHost::close() {
  if (auto r = ensure_window(); !r) return r;
  if (auto app = app_state_.lock()) {
    app->shutdown_started = true;
    app->run_exit_code = 0;
  }
  if (auto state = window_state_.lock()) {
    state->is_closed = true;
  }
  [(__bridge NSWindow*)window_ orderOut:nil];
  [NSApp stop:nil];
  return {};
}

Result<void> MacOSWindowHost::load_url(std::string_view) { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::load_file(std::string_view) { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::reload() { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::evaluate_script(std::string_view) { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::add_init_script(std::string_view) { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::open_devtools() { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::close_devtools() { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::on_page_load(PageLoadHandler) { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::set_navigation_handler(NavigationHandler) { return tl::unexpected(Error{"unsupported_by_backend", unsupported_webview_message()}); }
Result<void> MacOSWindowHost::register_command(std::string, CommandHandler) { return tl::unexpected(Error{"unsupported_by_backend", "macOS bridge backend not implemented yet"}); }
Result<void> MacOSWindowHost::emit(std::string, const Json&) { return tl::unexpected(Error{"bridge_unavailable", "macOS bridge backend not implemented yet"}); }

Result<Capabilities> MacOSWindowHost::capabilities() const {
  Capabilities caps;
  caps.window.borderless = true;
  caps.window.always_on_top = true;
  return caps;
}

} // namespace viewshell

#endif
