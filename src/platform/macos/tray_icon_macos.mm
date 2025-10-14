#include <optional>
#include "../../foundation/geometry.h"
#include "../../image.h"
#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

// Note: This file assumes ARC (Automatic Reference Counting) is enabled
// for proper memory management of Objective-C objects.

// Forward declarations
typedef void (^TrayIconClickedBlock)(nativeapi::TrayIconId tray_icon_id, const std::string& button);
typedef void (^TrayIconRightClickedBlock)(nativeapi::TrayIconId tray_icon_id);
typedef void (^TrayIconDoubleClickedBlock)(nativeapi::TrayIconId tray_icon_id);

// Key for associated object to store tray icon ID
static const void* kTrayIconIdKey = &kTrayIconIdKey;

@interface NSStatusBarButtonTarget : NSObject
@property(nonatomic, copy) TrayIconClickedBlock leftClickedBlock;
@property(nonatomic, copy) TrayIconRightClickedBlock rightClickedBlock;
@property(nonatomic, copy) TrayIconDoubleClickedBlock doubleClickedBlock;
@property(nonatomic, assign) nativeapi::TrayIcon* tray_icon;
- (void)statusItemClicked:(id)sender;
@end

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  std::shared_ptr<Image> image_;

  Impl(NSStatusItem* status_item)
      : ns_status_item_(status_item),
        ns_status_bar_button_target_(nil),
        menu_closed_listener_id_(0) {
    if (status_item) {
      // Check if ID already exists in the associated object
      NSNumber* allocated_id = objc_getAssociatedObject(status_item, kTrayIconIdKey);
      if (allocated_id) {
        // Reuse allocated ID
        id_ = [allocated_id longValue];
      } else {
        // Allocate new ID and store it
        id_ = IdAllocator::Allocate<TrayIcon>();
        objc_setAssociatedObject(status_item, kTrayIconIdKey, [NSNumber numberWithLong:id_],
                                 OBJC_ASSOCIATION_RETAIN_NONATOMIC);
      }

      // Create and set up button target
      ns_status_bar_button_target_ = [[NSStatusBarButtonTarget alloc] init];
      ns_status_bar_button_target_.tray_icon = nullptr;  // Will be set later

      // Set up click handlers
      [status_item.button setTarget:ns_status_bar_button_target_];
      [status_item.button setAction:@selector(statusItemClicked:)];

      // Enable right-click handling
      [status_item.button sendActionOn:NSEventMaskLeftMouseUp | NSEventMaskRightMouseUp];
    }
  }

  ~Impl() {
    // Remove the menu closed listener before cleaning up
    if (context_menu_ && menu_closed_listener_id_ != 0) {
      context_menu_->RemoveListener(menu_closed_listener_id_);
      menu_closed_listener_id_ = 0;
    }

    // Clean up blocks first
    if (ns_status_bar_button_target_) {
      ns_status_bar_button_target_.leftClickedBlock = nil;
      ns_status_bar_button_target_.rightClickedBlock = nil;
      ns_status_bar_button_target_.doubleClickedBlock = nil;
      ns_status_bar_button_target_.tray_icon = nullptr;
      ns_status_bar_button_target_ = nil;
    }

    // Then clean up the status item
    if (ns_status_item_) {
      // Remove target and action to prevent callbacks after destruction
      if (ns_status_item_.button) {
        [ns_status_item_.button setTarget:nil];
        [ns_status_item_.button setAction:nil];
      }
      // Clear menu reference
      ns_status_item_.menu = nil;

      // Clean up associated object
      objc_setAssociatedObject(ns_status_item_, kTrayIconIdKey, nil,
                               OBJC_ASSOCIATION_RETAIN_NONATOMIC);

      [[NSStatusBar systemStatusBar] removeStatusItem:ns_status_item_];
      ns_status_item_ = nil;
    }

    // Finally, safely clean up context_menu_ after all UI references are cleared
    if (context_menu_) {
      context_menu_.reset();  // Explicitly reset shared_ptr
    }
  }

  NSStatusItem* ns_status_item_;
  NSStatusBarButtonTarget* ns_status_bar_button_target_;

  TrayIconId id_;
  std::shared_ptr<Menu> context_menu_;
  size_t menu_closed_listener_id_;
};

TrayIcon::TrayIcon() : TrayIcon(nullptr) {}

TrayIcon::TrayIcon(void* tray) {
  NSStatusItem* status_item = nullptr;

  if (tray == nullptr) {
    // Create platform-specific NSStatusItem
    NSStatusBar* status_bar = [NSStatusBar systemStatusBar];
    status_item = [status_bar statusItemWithLength:NSVariableStatusItemLength];
  } else {
    status_item = (__bridge NSStatusItem*)tray;
  }

  // Initialize the Impl with the status item
  pimpl_ = std::make_unique<Impl>(status_item);

  if (pimpl_->ns_status_bar_button_target_) {
    pimpl_->ns_status_bar_button_target_.tray_icon = this;

    // 设置默认的 Block 处理器，直接发送事件
    pimpl_->ns_status_bar_button_target_.leftClickedBlock =
        ^(TrayIconId tray_icon_id, const std::string& button) {
          try {
            EmitSync<TrayIconClickedEvent>(tray_icon_id, button);
          } catch (...) {
            // Protect against event emission exceptions
          }
        };

    pimpl_->ns_status_bar_button_target_.rightClickedBlock = ^(TrayIconId tray_icon_id) {
      try {
        EmitSync<TrayIconRightClickedEvent>(tray_icon_id);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };

    pimpl_->ns_status_bar_button_target_.doubleClickedBlock = ^(TrayIconId tray_icon_id) {
      try {
        EmitSync<TrayIconDoubleClickedEvent>(tray_icon_id);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };
  }
}

TrayIcon::~TrayIcon() {
  // Clear the button target's reference to this object before destruction
  if (pimpl_ && pimpl_->ns_status_bar_button_target_) {
    pimpl_->ns_status_bar_button_target_.tray_icon = nullptr;
  }
}

TrayIconId TrayIcon::GetId() {
  return pimpl_->id_;
}

void TrayIcon::SetIcon(std::shared_ptr<Image> image) {
  if (!pimpl_->ns_status_item_ || !pimpl_->ns_status_item_.button) {
    return;
  }

  // Store the image reference
  pimpl_->image_ = image;

  NSImage* ns_image = nil;

  if (image) {
    // Get NSImage directly from Image object using GetNativeObject
    ns_image = (__bridge NSImage*)image->GetNativeObject();
  }

  if (ns_image) {
    // Set appropriate size for status bar
    [ns_image setSize:NSMakeSize(18, 18)];
    // Make it template image for proper appearance in dark mode
    [ns_image setTemplate:YES];

    // Set the image to the button
    [pimpl_->ns_status_item_.button setImage:ns_image];
  } else {
    // Clear the image if no valid icon is provided
    [pimpl_->ns_status_item_.button setImage:nil];
  }
}

std::shared_ptr<Image> TrayIcon::GetIcon() const {
  return pimpl_->image_;
}

void TrayIcon::SetTitle(std::optional<std::string> title) {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    if (title.has_value()) {
      NSString* title_string = [NSString stringWithUTF8String:title.value().c_str()];
      [pimpl_->ns_status_item_.button setTitle:title_string];
    } else {
      [pimpl_->ns_status_item_.button setTitle:@""];
    }
  }
}

std::optional<std::string> TrayIcon::GetTitle() {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSString* title_string = [pimpl_->ns_status_item_.button title];
    if (title_string && [title_string length] > 0) {
      return std::string([title_string UTF8String]);
    }
  }
  return std::nullopt;
}

void TrayIcon::SetTooltip(std::optional<std::string> tooltip) {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    if (tooltip.has_value()) {
      NSString* tooltip_string = [NSString stringWithUTF8String:tooltip.value().c_str()];
      [pimpl_->ns_status_item_.button setToolTip:tooltip_string];
    } else {
      [pimpl_->ns_status_item_.button setToolTip:nil];
    }
  }
}

std::optional<std::string> TrayIcon::GetTooltip() {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSString* tooltip_string = [pimpl_->ns_status_item_.button toolTip];
    if (tooltip_string && [tooltip_string length] > 0) {
      return std::string([tooltip_string UTF8String]);
    }
  }
  return std::nullopt;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  // Remove previous menu listener if it exists
  if (pimpl_->context_menu_ && pimpl_->menu_closed_listener_id_ != 0) {
    pimpl_->context_menu_->RemoveListener(pimpl_->menu_closed_listener_id_);
    pimpl_->menu_closed_listener_id_ = 0;
  }

  // Store the menu reference
  // Don't set the menu directly to the status item, as this would cause
  // macOS to take over click handling and prevent our custom click events
  // Instead, we'll show the menu manually in our click handler
  pimpl_->context_menu_ = menu;

  if (pimpl_->context_menu_) {
    auto pimpl_raw = pimpl_.get();
    pimpl_->menu_closed_listener_id_ = pimpl_->context_menu_->AddListener<MenuClosedEvent>(
        [pimpl_raw](const MenuClosedEvent& event) {
          if (pimpl_raw && pimpl_raw->ns_status_item_) {
            pimpl_raw->ns_status_item_.menu = nil;
          }
        });
  }
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSStatusBarButton* button = pimpl_->ns_status_item_.button;
    NSRect button_frame = button.frame;
    NSRect screen_frame = [button convertRect:button_frame toView:nil];
    NSRect window_frame = [button.window convertRectToScreen:screen_frame];

    // Convert from Cocoa coordinates (bottom-left origin) to standard coordinates (top-left origin)
    NSScreen* screen = [NSScreen mainScreen];
    CGFloat screen_height = screen.frame.size.height;

    bounds.x = window_frame.origin.x;
    bounds.y = screen_height - window_frame.origin.y - window_frame.size.height;
    bounds.width = window_frame.size.width;
    bounds.height = window_frame.size.height;
  }

  return bounds;
}

bool TrayIcon::SetVisible(bool visible) {
  if (!pimpl_->ns_status_item_) {
    return false;
  }

  [pimpl_->ns_status_item_ setVisible:visible ? YES : NO];
  return true;
}

bool TrayIcon::IsVisible() {
  if (pimpl_->ns_status_item_) {
    return [pimpl_->ns_status_item_ isVisible] == YES;
  }
  return false;
}

bool TrayIcon::OpenContextMenu(double x, double y) {
  if (!pimpl_->context_menu_) {
    return false;
  }

  // Open the context menu at the specified coordinates
  return pimpl_->context_menu_->Open(x, y);
}

bool TrayIcon::OpenContextMenu() {
  if (!pimpl_->context_menu_ || !pimpl_->ns_status_item_ || !pimpl_->ns_status_item_.button) {
    return false;
  }

  // Use the Swift approach: set menu to status item and simulate click
  // Get the native NSMenu object from our Menu wrapper
  NSMenu* native_menu = (__bridge NSMenu*)pimpl_->context_menu_->GetNativeObject();
  if (!native_menu) {
    return false;
  }

  //  // Set our menu delegate to handle menu close events
  //  [nativeMenu setDelegate:pimpl_->menu_delegate_];

  // Set the menu to the status item (like Swift version)
  pimpl_->ns_status_item_.menu = native_menu;

  // Simulate a click to show the menu (like Swift version)
  [pimpl_->ns_status_item_.button performClick:nil];

  return true;
}

bool TrayIcon::CloseContextMenu() {
  if (!pimpl_->context_menu_) {
    return true;  // No menu to close, consider success
  }

  // Close the context menu
  return pimpl_->context_menu_->Close();
}

void* TrayIcon::GetNativeObjectInternal() const {
  return (__bridge void*)pimpl_->ns_status_item_;
}

}  // namespace nativeapi

// Implementation of NSStatusBarButtonTarget
@implementation NSStatusBarButtonTarget

- (void)statusItemClicked:(id)sender {
  // Check if tray_icon is still valid before proceeding
  if (!self.tray_icon)
    return;

  // Create a local reference to prevent race conditions
  nativeapi::TrayIcon* tray_icon = self.tray_icon;
  if (!tray_icon)
    return;

  NSEvent* event = [NSApp currentEvent];
  if (!event)
    return;

  // Check the type of click and call appropriate block
  if (event.type == NSEventTypeRightMouseUp ||
      (event.type == NSEventTypeLeftMouseUp &&
       (event.modifierFlags & NSEventModifierFlagControl))) {
    // Right click or Ctrl+Left click
    if (_rightClickedBlock) {
      _rightClickedBlock(tray_icon->GetId());
    }
  } else if (event.type == NSEventTypeLeftMouseUp) {
    // Check for double click
    if (event.clickCount == 2) {
      if (_doubleClickedBlock) {
        _doubleClickedBlock(tray_icon->GetId());
      }
    } else {
      if (_leftClickedBlock) {
        _leftClickedBlock(tray_icon->GetId(), "left");
      }
    }
  }
}

@end
