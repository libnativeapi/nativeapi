#include "../../tray_icon.h"
#include "../../menu.h"
#include "../../geometry.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

// Forward declarations
@interface TrayIconDelegate : NSObject
@property (nonatomic, assign) nativeapi::TrayIcon* trayIcon;
- (void)statusItemClicked:(id)sender;
- (void)statusItemRightClicked:(id)sender;
@end

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl() : ns_status_item_(nil), delegate_(nil), visible_(false) {}

  Impl(NSStatusItem* status_item) : ns_status_item_(status_item), delegate_(nil), visible_(false) {
    if (status_item) {
      // Create and set up delegate
      delegate_ = [[TrayIconDelegate alloc] init];
      delegate_.trayIcon = nullptr; // Will be set later

      // Set up click handlers
      [status_item.button setTarget:delegate_];
      [status_item.button setAction:@selector(statusItemClicked:)];

      // Enable right-click handling
      [status_item.button sendActionOn:NSEventMaskLeftMouseUp | NSEventMaskRightMouseUp];
    }
  }

  ~Impl() {
    if (ns_status_item_) {
      [[NSStatusBar systemStatusBar] removeStatusItem:ns_status_item_];
      ns_status_item_ = nil;
    }
    if (delegate_) {
      delegate_ = nil;
    }
  }

  NSStatusItem* ns_status_item_;
  TrayIconDelegate* delegate_;
  std::shared_ptr<Menu> context_menu_;
  std::string title_;
  std::string tooltip_;
  bool visible_;

  // Event callbacks
  std::function<void()> on_left_click_;
  std::function<void()> on_right_click_;
  std::function<void()> on_double_click_;
};

TrayIcon::TrayIcon() : pimpl_(new Impl()) {
  id = -1;
  if (pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = this;
  }
}

TrayIcon::TrayIcon(void* tray) : pimpl_(new Impl((__bridge NSStatusItem*)tray)) {
  id = -1; // Will be set by TrayManager when created
  if (pimpl_->delegate_) {
    pimpl_->delegate_.trayIcon = this;
  }
}

TrayIcon::~TrayIcon() {
  delete pimpl_;
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
  pimpl_->context_menu_ = menu;
  // Note: Menu functionality is not implemented in this simplified version
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

void TrayIcon::SetOnLeftClick(std::function<void()> callback) {
  pimpl_->on_left_click_ = callback;
}

void TrayIcon::SetOnRightClick(std::function<void()> callback) {
  pimpl_->on_right_click_ = callback;
}

void TrayIcon::SetOnDoubleClick(std::function<void()> callback) {
  pimpl_->on_double_click_ = callback;
}

bool TrayIcon::ShowContextMenu(double x, double y) {
  // Note: Context menu functionality is not implemented in this simplified version
  return false;
}

bool TrayIcon::ShowContextMenu() {
  // Note: Context menu functionality is not implemented in this simplified version
  return false;
}

// Internal method to handle click events
void TrayIcon::HandleLeftClick() {
  if (pimpl_->on_left_click_) {
    pimpl_->on_left_click_();
  }
}

void TrayIcon::HandleRightClick() {
  if (pimpl_->on_right_click_) {
    pimpl_->on_right_click_();
  }
}

void TrayIcon::HandleDoubleClick() {
  if (pimpl_->on_double_click_) {
    pimpl_->on_double_click_();
  }
}

} // namespace nativeapi

// Implementation of TrayIconDelegate
@implementation TrayIconDelegate

- (void)statusItemClicked:(id)sender {
  if (!_trayIcon) return;

  NSEvent* event = [NSApp currentEvent];
  if (!event) return;

  // Check the type of click
  if (event.type == NSEventTypeRightMouseUp ||
      (event.type == NSEventTypeLeftMouseUp && (event.modifierFlags & NSEventModifierFlagControl))) {
    // Right click or Ctrl+Left click
    _trayIcon->HandleRightClick();
  } else if (event.type == NSEventTypeLeftMouseUp) {
    // Check for double click
    if (event.clickCount == 2) {
      _trayIcon->HandleDoubleClick();
    } else {
      _trayIcon->HandleLeftClick();
    }
  }
}

- (void)statusItemRightClicked:(id)sender {
  if (_trayIcon) {
    _trayIcon->HandleRightClick();
  }
}

@end
