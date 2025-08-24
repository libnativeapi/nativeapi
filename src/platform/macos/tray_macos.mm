#include <iostream>
#include "../../menu.h"
#include "../../tray.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Private implementation class
class Tray::Impl {
 public:
  Impl(NSStatusItem* tray) : ns_status_item_(tray) {}
  NSStatusItem* ns_status_item_;
  Menu context_menu_;  // 添加菜单成员变量来保持菜单对象的生命周期
};

Tray::Tray() : pimpl_(new Impl(nil)) {
  id = -1;
}

Tray::Tray(void* tray) : pimpl_(new Impl((__bridge NSStatusItem*)tray)) {
  id = -1;  // Will be set by TrayManager when created
}

Tray::~Tray() {
  delete pimpl_;
}

void Tray::SetIcon(std::string icon) {
  // Check if the icon is a base64 string
  if (icon.find("data:image") != std::string::npos) {
    // Extract the base64 part
    size_t pos = icon.find("base64,");
    if (pos != std::string::npos) {
      std::string base64Icon = icon.substr(pos + 7);

      // Convert base64 to NSData
      NSData* imageData = [[NSData alloc]
          initWithBase64EncodedString:[NSString stringWithUTF8String:base64Icon.c_str()]
                              options:NSDataBase64DecodingIgnoreUnknownCharacters];

      // Create image from data
      NSImage* image = [[NSImage alloc] initWithData:imageData];

      // Set image size and template property
      [image setSize:NSMakeSize(20, 20)];  // Default icon size
      [image setTemplate:YES];

      // Set the image to the button
      [pimpl_->ns_status_item_.button setImage:image];
    }
  } else {
    // Use the icon as a file path or named image
    [pimpl_->ns_status_item_.button
        setImage:[NSImage imageNamed:[NSString stringWithUTF8String:icon.c_str()]]];
  }
}

void Tray::SetTitle(std::string title) {
  if (pimpl_->ns_status_item_.button) {
    [pimpl_->ns_status_item_.button setTitle:[NSString stringWithUTF8String:title.c_str()]];
  }
}

std::string Tray::GetTitle() {
  return [[pimpl_->ns_status_item_.button title] UTF8String];
}

void Tray::SetTooltip(std::string tooltip) {
  [pimpl_->ns_status_item_.button setToolTip:[NSString stringWithUTF8String:tooltip.c_str()]];
}

std::string Tray::GetTooltip() {
  return [[pimpl_->ns_status_item_.button toolTip] UTF8String];
}

void Tray::SetContextMenu(Menu menu) {
  // 保存菜单对象的副本来维持其生命周期
  pimpl_->context_menu_ = menu;

  NSMenu* ns_menu = (__bridge NSMenu*)pimpl_->context_menu_.GetNativeMenu();
  if (ns_menu) {
    [pimpl_->ns_status_item_ setMenu:ns_menu];
  }
}

Menu Tray::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle Tray::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  if (pimpl_->ns_status_item_.button) {
    NSRect buttonFrame = [pimpl_->ns_status_item_.button frame];
    NSRect screenFrame = [pimpl_->ns_status_item_.button convertRect:buttonFrame toView:nil];
    NSRect windowFrame = [pimpl_->ns_status_item_.button.window convertRectToScreen:screenFrame];

    bounds.x = windowFrame.origin.x;
    bounds.y = windowFrame.origin.y;
    bounds.width = windowFrame.size.width;
    bounds.height = windowFrame.size.height;
  }

  return bounds;
}
}  // namespace nativeapi
