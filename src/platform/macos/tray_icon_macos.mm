#include "../../tray_icon.h"
#include "../../menu.h"
#include "../../geometry.h"
#include "../../tray_icon_event.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

// Note: This file assumes ARC (Automatic Reference Counting) is enabled
// for proper memory management of Objective-C objects.

// Forward declarations
@interface TrayIconDelegate : NSObject
@property (nonatomic, assign) nativeapi::TrayIcon* trayIcon;
- (void)statusItemClicked:(id)sender;
- (void)statusItemRightClicked:(id)sender;
@end

@interface TrayIconMenuDelegate : NSObject <NSMenuDelegate>
@property (nonatomic, assign) nativeapi::TrayIcon* trayIcon;
@end

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl() : ns_status_item_(nil), delegate_(nil), menu_delegate_(nil), visible_(false) {}

  Impl(NSStatusItem* status_item) : ns_status_item_(status_item), delegate_(nil), menu_delegate_(nil), visible_(false) {
    if (status_item) {
      // Create and set up delegate
      delegate_ = [[TrayIconDelegate alloc] init];
      delegate_.trayIcon = nullptr; // Will be set later

      // Create menu delegate
      menu_delegate_ = [[TrayIconMenuDelegate alloc] init];
      menu_delegate_.trayIcon = nullptr; // Will be set later

      // Set up click handlers
      [status_item.button setTarget:delegate_];
      [status_item.button setAction:@selector(statusItemClicked:)];

      // Enable right-click handling
      [status_item.button sendActionOn:NSEventMaskLeftMouseUp | NSEventMaskRightMouseUp];
    }
  }

  ~Impl() {
    // First, clean up delegates to prevent callbacks
    if (delegate_) {
      delegate_.trayIcon = nullptr;  // Clear the raw pointer first
      delegate_ = nil;
    }

    if (menu_delegate_) {
      menu_delegate_.trayIcon = nullptr;  // Clear the raw pointer first
      menu_delegate_ = nil;
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
  TrayIconMenuDelegate* menu_delegate_;
  std::shared_ptr<Menu> context_menu_;
  std::string title_;
  std::string tooltip_;
  bool visible_;
};

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>()) {
  id = -1;
  if (pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = this;
  }
  if (pimpl_->menu_delegate_) {
    pimpl_->menu_delegate_.trayIcon = this;
  }
}

TrayIcon::TrayIcon(void* tray) : pimpl_(std::make_unique<Impl>((__bridge NSStatusItem*)tray)) {
  id = -1; // Will be set by TrayManager when created
  if (pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = this;
  }
  if (pimpl_->menu_delegate_) {
    pimpl_->menu_delegate_.trayIcon = this;
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
      NSData* imageData = [[NSData alloc]
          initWithBase64EncodedString:base64String
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

void TrayIcon::SetTitle(std::string title) {
  pimpl_->title_ = title;

  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSString* titleString = [NSString stringWithUTF8String:title.c_str()];
    [pimpl_->ns_status_item_.button setTitle:titleString];
  }
}

std::string TrayIcon::GetTitle() {
  return pimpl_->title_;
}

void TrayIcon::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;

  if (pimpl_->ns_status_item_ && pimpl_->ns_status_item_.button) {
    NSString* tooltipString = [NSString stringWithUTF8String:tooltip.c_str()];
    [pimpl_->ns_status_item_.button setToolTip:tooltipString];
  }
}

std::string TrayIcon::GetTooltip() {
  return pimpl_->tooltip_;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  // Store the menu reference
  // Don't set the menu directly to the status item, as this would cause
  // macOS to take over click handling and prevent our custom click events
  // Instead, we'll show the menu manually in our click handler
  pimpl_->context_menu_ = menu;
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

bool TrayIcon::Show() {
  if (pimpl_->ns_status_item_) {
    [pimpl_->ns_status_item_ setVisible:YES];
    pimpl_->visible_ = true;
    return true;
  }
  return false;
}

bool TrayIcon::Hide() {
  if (pimpl_->ns_status_item_) {
    [pimpl_->ns_status_item_ setVisible:NO];
    pimpl_->visible_ = false;
    return true;
  }
  return false;
}

bool TrayIcon::IsVisible() {
  if (pimpl_->ns_status_item_) {
    return [pimpl_->ns_status_item_ isVisible] == YES;
  }
  return pimpl_->visible_;
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

  // Set our menu delegate to handle menu close events
  [nativeMenu setDelegate:pimpl_->menu_delegate_];

  // Set the menu to the status item (like Swift version)
  pimpl_->ns_status_item_.menu = nativeMenu;

  // Simulate a click to show the menu (like Swift version)
  [pimpl_->ns_status_item_.button performClick:nil];

  return true;
}

bool TrayIcon::CloseContextMenu() {
  if (!pimpl_->context_menu_) {
    return true; // No menu to close, consider success
  }

  // Close the context menu
  return pimpl_->context_menu_->Close();
}

// Internal method to handle click events
void TrayIcon::HandleLeftClick() {
  try {
    EmitSync<TrayIconClickedEvent>(id, "left");
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::HandleRightClick() {
  try {
    EmitSync<TrayIconRightClickedEvent>(id);
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::HandleDoubleClick() {
  try {
    EmitSync<TrayIconDoubleClickedEvent>(id);
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::ClearStatusItemMenu() {
  if (pimpl_->ns_status_item_) {
    pimpl_->ns_status_item_.menu = nil;
  }
}

} // namespace nativeapi

// Implementation of TrayIconDelegate
@implementation TrayIconDelegate

- (void)statusItemClicked:(id)sender {
  // Check if trayIcon is still valid before proceeding
  if (!_trayIcon) return;

  // Create a local reference to prevent race conditions
  nativeapi::TrayIcon* trayIcon = _trayIcon;
  if (!trayIcon) return;

  NSEvent* event = [NSApp currentEvent];
  if (!event) return;

  // Check the type of click
  if (event.type == NSEventTypeRightMouseUp ||
      (event.type == NSEventTypeLeftMouseUp && (event.modifierFlags & NSEventModifierFlagControl))) {
    // Right click or Ctrl+Left click
    trayIcon->HandleRightClick();
  } else if (event.type == NSEventTypeLeftMouseUp) {
    // Check for double click
    if (event.clickCount == 2) {
      trayIcon->HandleDoubleClick();
    } else {
      trayIcon->HandleLeftClick();
    }
  }
}

- (void)statusItemRightClicked:(id)sender {
  // Check if trayIcon is still valid before proceeding
  if (_trayIcon) {
    _trayIcon->HandleRightClick();
  }
}

@end

// Implementation of TrayIconMenuDelegate
@implementation TrayIconMenuDelegate

- (void)menuDidClose:(NSMenu *)menu {
  // Check if trayIcon is still valid before proceeding
  if (_trayIcon) {
    // Call a public method to clear the menu (we'll add this method)
    _trayIcon->ClearStatusItemMenu();
  }
}

@end
