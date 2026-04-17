#ifdef __APPLE__

#include "host/macos/macos_tray_host.h"

#import <AppKit/AppKit.h>

static NSString* to_nsstring(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
}

namespace viewshell { class MacOSTrayHost; }

@interface ViewshellTrayDelegate : NSObject
@property(nonatomic, assign) viewshell::MacOSTrayHost* host;
- (void)trayClicked:(id)sender;
- (void)menuItemClicked:(id)sender;
@end

@implementation ViewshellTrayDelegate

- (void)trayClicked:(id)sender {
  if (!self.host) return;

  NSEvent* event = [NSApp currentEvent];
  if (event.type == NSEventTypeRightMouseUp) {
    // Right click: show popup menu
    NSStatusItem* item = (__bridge NSStatusItem*)self.host->status_item_ptr();
    NSMenu* menu = (__bridge NSMenu*)self.host->menu_ptr();
    if (item && menu) {
      [item popUpStatusItemMenu:menu];
    }
  } else {
    // Left click: call on_click callback
    self.host->invoke_click();
  }
}

- (void)menuItemClicked:(id)sender {
  if (!self.host) return;
  NSMenuItem* item = (NSMenuItem*)sender;
  NSInteger tag = [item tag];
  self.host->invoke_menu_click(tag);
}

@end

namespace viewshell {

// Public accessors for the delegate
void* MacOSTrayHost::status_item_ptr() const { return status_item_; }
void* MacOSTrayHost::menu_ptr() const { return menu_; }

void MacOSTrayHost::invoke_click() {
  if (on_click_) on_click_();
}

void MacOSTrayHost::invoke_menu_click(int index) {
  if (on_menu_click_ && index >= 0 && index < static_cast<int>(menu_items_.size())) {
    on_menu_click_(menu_items_[index].id);
  }
}

MacOSTrayHost::~MacOSTrayHost() {
  if (status_item_) {
    [[NSStatusBar systemStatusBar] removeStatusItem:(NSStatusItem*)status_item_];
    [(NSStatusItem*)status_item_ release];
  }
  if (menu_) {
    [(NSMenu*)menu_ release];
  }
  if (delegate_) {
    [(ViewshellTrayDelegate*)delegate_ release];
  }
}

Result<std::shared_ptr<MacOSTrayHost>> MacOSTrayHost::create(const TrayOptions& options) {
  auto host = std::shared_ptr<MacOSTrayHost>(new MacOSTrayHost());
  host->on_click_ = options.on_click;
  host->on_menu_click_ = options.on_menu_click;
  host->menu_items_ = options.menu;

  NSStatusItem* status_item = [[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength];
  if (!status_item) {
    return tl::unexpected(Error{"tray_creation_failed", "failed to create NSStatusItem"});
  }

  ViewshellTrayDelegate* delegate = [[ViewshellTrayDelegate alloc] init];
  delegate.host = host.get();

  NSStatusBarButton* button = [status_item button];

  // Load icon
  if (!options.icon_path.empty()) {
    NSImage* image = [[NSImage alloc] initWithContentsOfFile:to_nsstring(options.icon_path)];
    if (image) {
      [image setTemplate:YES];
      [button setImage:image];
      [image release];
    }
  }

  // Set tooltip
  if (!options.tooltip.empty()) {
    [button setToolTip:to_nsstring(options.tooltip)];
  }

  // Configure button action for left and right clicks
  [button setTarget:delegate];
  [button setAction:@selector(trayClicked:)];
  [button sendActionOn:(NSEventMaskLeftMouseUp | NSEventMaskRightMouseUp)];

  host->status_item_ = (void*)[status_item retain];
  host->delegate_ = (void*)[delegate retain];

  // Build menu
  host->rebuild_menu();

  return host;
}

void MacOSTrayHost::rebuild_menu() {
  if (menu_) {
    [(NSMenu*)menu_ release];
    menu_ = nullptr;
  }

  NSMenu* menu = [[NSMenu alloc] init];
  [menu setAutoenablesItems:NO];

  ViewshellTrayDelegate* delegate = (ViewshellTrayDelegate*)delegate_;

  for (int i = 0; i < static_cast<int>(menu_items_.size()); ++i) {
    const auto& item = menu_items_[i];
    if (item.label.empty()) {
      [menu addItem:[NSMenuItem separatorItem]];
    } else {
      NSMenuItem* ns_item = [[NSMenuItem alloc] initWithTitle:to_nsstring(item.label)
                                                       action:@selector(menuItemClicked:)
                                                keyEquivalent:@""];
      [ns_item setTarget:delegate];
      [ns_item setTag:i];
      [ns_item setEnabled:item.enabled ? YES : NO];
      [menu addItem:ns_item];
      [ns_item release];
    }
  }

  menu_ = (void*)[menu retain];
}

Result<void> MacOSTrayHost::set_icon(std::string_view icon_path) {
  if (!status_item_) {
    return tl::unexpected(Error{"invalid_state", "tray is not available"});
  }
  NSStatusBarButton* button = [(NSStatusItem*)status_item_ button];
  NSImage* image = [[NSImage alloc] initWithContentsOfFile:to_nsstring(icon_path)];
  if (image) {
    [image setTemplate:YES];
    [button setImage:image];
    [image release];
  }
  return {};
}

Result<void> MacOSTrayHost::set_tooltip(std::string_view tooltip) {
  if (!status_item_) {
    return tl::unexpected(Error{"invalid_state", "tray is not available"});
  }
  NSStatusBarButton* button = [(NSStatusItem*)status_item_ button];
  [button setToolTip:to_nsstring(tooltip)];
  return {};
}

Result<void> MacOSTrayHost::set_menu(std::vector<TrayMenuItem> menu) {
  menu_items_ = std::move(menu);
  rebuild_menu();
  return {};
}

Result<Geometry> MacOSTrayHost::get_icon_rect() const {
  if (!status_item_) {
    return tl::unexpected(Error{"invalid_state", "tray is not available"});
  }
  NSStatusItem* item = (NSStatusItem*)status_item_;
  NSWindow* window = [[item button] window];
  if (!window) {
    return tl::unexpected(Error{"icon_rect_failed",
        "status item window is not available"});
  }
  NSRect frame = [window frame];
  int screen_height = static_cast<int>([[NSScreen mainScreen] frame].size.height);
  int flipped_y = screen_height - static_cast<int>(frame.origin.y) - static_cast<int>(frame.size.height);
  return Geometry{
      static_cast<int>(frame.origin.x),
      flipped_y,
      static_cast<int>(frame.size.width),
      static_cast<int>(frame.size.height)};
}

Result<void> MacOSTrayHost::remove() {
  if (status_item_) {
    [[NSStatusBar systemStatusBar] removeStatusItem:(NSStatusItem*)status_item_];
    [(NSStatusItem*)status_item_ release];
    status_item_ = nullptr;
  }
  if (menu_) {
    [(NSMenu*)menu_ release];
    menu_ = nullptr;
  }
  if (delegate_) {
    [(ViewshellTrayDelegate*)delegate_ release];
    delegate_ = nullptr;
  }
  return {};
}

} // namespace viewshell

#endif
