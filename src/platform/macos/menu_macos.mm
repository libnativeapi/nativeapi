#include <iostream>
#include "../../menu.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Private implementation class
class MenuItem::Impl {
 public:
  Impl(NSMenuItem* menu_item) : ns_menu_item_(menu_item) {}
  NSMenuItem* ns_menu_item_;
};

MenuItem::MenuItem() : pimpl_(new Impl([[NSMenuItem alloc] init])) {
  id = -1;
}

MenuItem::MenuItem(void* menu_item) : pimpl_(new Impl((__bridge NSMenuItem*)menu_item)) {
  id = 0;
}

MenuItem::~MenuItem() {
  delete pimpl_;
}

void MenuItem::SetTitle(std::string title) {
  [pimpl_->ns_menu_item_ setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

std::string MenuItem::GetTitle() {
  return [[pimpl_->ns_menu_item_ title] UTF8String];
}

void MenuItem::SetIcon(std::string icon) {
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

      // Set image size
      [image setSize:NSMakeSize(16, 16)];  // Default menu item icon size
      [image setTemplate:YES];

      // Set the image to the menu item
      [pimpl_->ns_menu_item_ setImage:image];
    }
  } else {
    // Use the icon as a file path or named image
    NSImage* image = [NSImage imageNamed:[NSString stringWithUTF8String:icon.c_str()]];
    if (image) {
      [image setSize:NSMakeSize(16, 16)];
      [image setTemplate:YES];
      [pimpl_->ns_menu_item_ setImage:image];
    }
  }
}

std::string MenuItem::GetIcon() {
  NSImage* image = [pimpl_->ns_menu_item_ image];
  if (image) {
    // For simplicity, return the image name if it's a named image
    // In a real implementation, you might want to return the actual image data
    return "image_set";
  }
  return "";
}

void MenuItem::SetTooltip(std::string tooltip) {
  [pimpl_->ns_menu_item_ setToolTip:[NSString stringWithUTF8String:tooltip.c_str()]];
}

std::string MenuItem::GetTooltip() {
  NSString* tooltip = [pimpl_->ns_menu_item_ toolTip];
  return tooltip ? [tooltip UTF8String] : "";
}

// Private implementation class
class Menu::Impl {
 public:
  Impl(NSMenu* menu) : ns_menu_(menu) {}
  NSMenu* ns_menu_;
};

Menu::Menu() : pimpl_(new Impl([[NSMenu alloc] init])) {
  id = -1;
}

Menu::Menu(void* menu) : pimpl_(new Impl((__bridge NSMenu*)menu)) {
  id = 0;
}

Menu::~Menu() {
  delete pimpl_;
}

void Menu::AddItem(MenuItem item) {
  if (pimpl_->ns_menu_ && item.pimpl_->ns_menu_item_) {
    [pimpl_->ns_menu_ addItem:item.pimpl_->ns_menu_item_];
  }
}

void Menu::RemoveItem(MenuItem item) {
  if (pimpl_->ns_menu_ && item.pimpl_->ns_menu_item_) {
    [pimpl_->ns_menu_ removeItem:item.pimpl_->ns_menu_item_];
  }
}

void Menu::AddSeparator() {
  if (pimpl_->ns_menu_) {
    [pimpl_->ns_menu_ addItem:[NSMenuItem separatorItem]];
  }
}

MenuItem Menu::CreateItem(std::string title) {
  NSMenuItem* ns_menu_item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:title.c_str()]
                                                        action:nil
                                                 keyEquivalent:@""];
  return MenuItem((__bridge void*)ns_menu_item);
}

MenuItem Menu::CreateItem(std::string title, std::string icon) {
  MenuItem item = CreateItem(title);
  item.SetIcon(icon);
  return item;
}

void* Menu::GetNativeMenu() {
  return (__bridge void*)pimpl_->ns_menu_;
}

}  // namespace nativeapi
