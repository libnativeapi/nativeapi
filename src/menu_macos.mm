#include <iostream>
#include "menu.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Private implementation class
class MenuItem::Impl {
 public:
  Impl(NSMenuItem* menu_item) : ns_menu_item_(menu_item) {}
  NSMenuItem* ns_menu_item_;
};

MenuItem::MenuItem() : pimpl_(new Impl(nil)) {
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

// Private implementation class
class Menu::Impl {
 public:
  Impl(NSMenu* menu) : ns_menu_(menu) {}
  NSMenu* ns_menu_;
};

Menu::Menu() : pimpl_(new Impl(nil)) {
  id = -1;
}

Menu::Menu(void* menu) : pimpl_(new Impl((__bridge NSMenu*)menu)) {
  id = 0;
}

Menu::~Menu() {
  delete pimpl_;
}

void Menu::AddItem(MenuItem item) {
  // NSMenuItem* ns_menu_item = (__bridge NSMenuItem*)item.GetNativeMenuItem();
  // [pimpl_->ns_menu_ addItem:ns_menu_item];
}

void Menu::RemoveItem(MenuItem item) {
  // NSMenuItem* ns_menu_item = (__bridge NSMenuItem*)item.GetNativeMenuItem();
  // [pimpl_->ns_menu_ removeItem:ns_menu_item];
}


}  // namespace nativeapi
