#include "../../foundation/geometry.h"
#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

// Note: This file assumes ARC (Automatic Reference Counting) is enabled
// for proper memory management of Objective-C objects.

// Forward declarations
typedef void (^TrayIconClickedBlock)(nativeapi::TrayIconID tray_icon_id, const std::string& button);
typedef void (^TrayIconRightClickedBlock)(nativeapi::TrayIconID tray_icon_id);
typedef void (^TrayIconDoubleClickedBlock)(nativeapi::TrayIconID tray_icon_id);

@interface TrayIconDelegate : NSObject
@property(nonatomic, assign) nativeapi::TrayIcon* trayIcon;
@property(nonatomic, copy) TrayIconClickedBlock leftClickedBlock;
@property(nonatomic, copy) TrayIconRightClickedBlock rightClickedBlock;
@property(nonatomic, copy) TrayIconDoubleClickedBlock doubleClickedBlock;
- (void)statusItemClicked:(id)sender;
@end

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl() : ns_status_item_(nil), delegate_(nil) {}

  Impl(NSStatusItem* status_item) : ns_status_item_(status_item), delegate_(nil) {
    if (status_item) {
      // Create and set up delegate
      delegate_ = [[TrayIconDelegate alloc] init];
      delegate_.trayIcon = nullptr;  // Will be set later

      // Set up click handlers
      [status_item.button setTarget:delegate_];
      [status_item.button setAction:@selector(statusItemClicked:)];

      // Enable right-click handling
      [status_item.button sendActionOn:NSEventMaskLeftMouseUp | NSEventMaskRightMouseUp];
    }
  }

  ~Impl() {
    // Clean up blocks first
    if (delegate_) {
      delegate_.leftClickedBlock = nil;
      delegate_.rightClickedBlock = nil;
      delegate_.doubleClickedBlock = nil;
      delegate_.trayIcon = nullptr;
      delegate_ = nil;
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
      [[NSStatusBar systemStatusBar] removeStatusItem:ns_status_item_];
      ns_status_item_ = nil;
    }

    // Finally, safely clean up context_menu_ after all UI references are cleared
    if (context_menu_) {
      context_menu_.reset();  // Explicitly reset shared_ptr
    }
  }

  NSStatusItem* ns_status_item_;
  TrayIconDelegate* delegate_;
  std::shared_ptr<Menu> context_menu_;
};

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>()) {
  id = -1;
  if (pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = this;

    // 设置默认的 Block 处理器，直接发送事件
    pimpl_->delegate_.leftClickedBlock = ^(TrayIconID tray_icon_id, const std::string& button) {
      try {
        EmitSync<TrayIconClickedEvent>(tray_icon_id, button);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };

    pimpl_->delegate_.rightClickedBlock = ^(TrayIconID tray_icon_id) {
      try {
        EmitSync<TrayIconRightClickedEvent>(tray_icon_id);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };

    pimpl_->delegate_.doubleClickedBlock = ^(TrayIconID tray_icon_id) {
      try {
        EmitSync<TrayIconDoubleClickedEvent>(tray_icon_id);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };
  }
}

TrayIcon::TrayIcon(void* tray) : pimpl_(std::make_unique<Impl>((__bridge NSStatusItem*)tray)) {
  id = -1;  // Will be set by TrayManager when created
  if (pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = this;

    // 设置默认的 Block 处理器，直接发送事件
    pimpl_->delegate_.leftClickedBlock = ^(TrayIconID tray_icon_id, const std::string& button) {
      try {
        EmitSync<TrayIconClickedEvent>(tray_icon_id, button);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };

    pimpl_->delegate_.rightClickedBlock = ^(TrayIconID tray_icon_id) {
      try {
        EmitSync<TrayIconRightClickedEvent>(tray_icon_id);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };

    pimpl_->delegate_.doubleClickedBlock = ^(TrayIconID tray_icon_id) {
      try {
        EmitSync<TrayIconDoubleClickedEvent>(tray_icon_id);
      } catch (...) {
        // Protect against event emission exceptions
      }
    };
  }
}

TrayIcon::~TrayIcon() {
  // Clear the delegate's reference to this object before destruction
  if (pimpl_ && pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = nullptr;
  }
}

void TrayIcon::SetIcon(std::string icon) {
  if (!pimpl_->ns_status_item_ || !pimpl_->ns_status_item_.button) {
    return;
  }

  NSImage* image = nil;

  // Check if the icon is a base64 string
  if (icon.find("data:image") == 0) {
    // Extract the base64 part
    size_t pos = icon.find("base64,");
    if (pos != std::string::npos) {
      std::string base64Icon = icon.substr(pos + 7);

      // Convert base64 to NSData
      NSString* base64String = [NSString stringWithUTF8String:base64Icon.c_str()];
      NSData* imageData =
          [[NSData alloc] initWithBase64EncodedString:base64String
                                              options:NSDataBase64DecodingIgnoreUnknownCharacters];

      if (imageData) {
        // Create image from data
        image = [[NSImage alloc] initWithData:imageData];
      }
    }
  } else if (!icon.empty()) {
    // Try as file path first
    NSString* iconPath = [NSString stringWithUTF8String:icon.c_str()];
    image = [[NSImage alloc] initWithContentsOfFile:iconPath];

    // If file path failed, try as system image name
    if (!image) {
      image = [NSImage imageNamed:iconPath];
    }
  }

  if (image) {
    // Set appropriate size for status bar
    [image setSize:NSMakeSize(18, 18)];
    // Make it template image for proper appearance in dark mode
    [image setTemplate:YES];

    // Set the image to the button
    [pimpl_->ns_status_item_.button setImage:image];
  } else {
    // Clear the image if no valid icon is provided
    [pimpl_->ns_status_item_.button setImage:nil];
  }
}

void TrayIcon::SetTitle(std::optional<std::string> title) {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    if (title.has_value()) {
      NSString* titleString = [NSString stringWithUTF8String:title.value().c_str()];
      [pimpl_->ns_status_item_.button setTitle:titleString];
    } else {
      [pimpl_->ns_status_item_.button setTitle:@""];
    }
  }
}

std::optional<std::string> TrayIcon::GetTitle() {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSString* titleString = [pimpl_->ns_status_item_.button title];
    if (titleString && [titleString length] > 0) {
      return std::string([titleString UTF8String]);
    }
  }
  return std::nullopt;
}

void TrayIcon::SetTooltip(std::optional<std::string> tooltip) {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    if (tooltip.has_value()) {
      NSString* tooltipString = [NSString stringWithUTF8String:tooltip.value().c_str()];
      [pimpl_->ns_status_item_.button setToolTip:tooltipString];
    } else {
      [pimpl_->ns_status_item_.button setToolTip:nil];
    }
  }
}

std::optional<std::string> TrayIcon::GetTooltip() {
  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSString* tooltipString = [pimpl_->ns_status_item_.button toolTip];
    if (tooltipString && [tooltipString length] > 0) {
      return std::string([tooltipString UTF8String]);
    }
  }
  return std::nullopt;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  // Store the menu reference
  // Don't set the menu directly to the status item, as this would cause
  // macOS to take over click handling and prevent our custom click events
  // Instead, we'll show the menu manually in our click handler
  pimpl_->context_menu_ = menu;
  auto pimpl_raw = pimpl_.get();
  pimpl_->context_menu_->AddListener<MenuClosedEvent>([pimpl_raw](const MenuClosedEvent& event) {
    if (pimpl_raw && pimpl_raw->ns_status_item_) {
      pimpl_raw->ns_status_item_.menu = nil;
    }
  });
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSStatusBarButton* button = pimpl_->ns_status_item_.button;
    NSRect buttonFrame = button.frame;
    NSRect screenFrame = [button convertRect:buttonFrame toView:nil];
    NSRect windowFrame = [button.window convertRectToScreen:screenFrame];

    // Convert from Cocoa coordinates (bottom-left origin) to standard coordinates (top-left origin)
    NSScreen* screen = [NSScreen mainScreen];
    CGFloat screenHeight = screen.frame.size.height;

    bounds.x = windowFrame.origin.x;
    bounds.y = screenHeight - windowFrame.origin.y - windowFrame.size.height;
    bounds.width = windowFrame.size.width;
    bounds.height = windowFrame.size.height;
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
  NSMenu* nativeMenu = (__bridge NSMenu*)pimpl_->context_menu_->GetNativeObject();
  if (!nativeMenu) {
    return false;
  }

  //  // Set our menu delegate to handle menu close events
  //  [nativeMenu setDelegate:pimpl_->menu_delegate_];

  // Set the menu to the status item (like Swift version)
  pimpl_->ns_status_item_.menu = nativeMenu;

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

// Implementation of TrayIconDelegate
@implementation TrayIconDelegate

- (void)statusItemClicked:(id)sender {
  // Check if trayIcon is still valid before proceeding
  if (!_trayIcon)
    return;

  // Create a local reference to prevent race conditions
  nativeapi::TrayIcon* trayIcon = _trayIcon;
  if (!trayIcon)
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
      _rightClickedBlock(trayIcon->id);
    }
  } else if (event.type == NSEventTypeLeftMouseUp) {
    // Check for double click
    if (event.clickCount == 2) {
      if (_doubleClickedBlock) {
        _doubleClickedBlock(trayIcon->id);
      }
    } else {
      if (_leftClickedBlock) {
        _leftClickedBlock(trayIcon->id, "left");
      }
    }
  }
}

@end
