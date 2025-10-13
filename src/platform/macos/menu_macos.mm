#include <atomic>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>
#include "../../menu.h"
#include "../../menu_event.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

// Note: This file assumes ARC (Automatic Reference Counting) is enabled
// for proper memory management of Objective-C objects.

// Static keys for associated objects
static const void* kMenuItemIdKey = &kMenuItemIdKey;
static const void* kMenuIdKey = &kMenuIdKey;

// Forward declarations - moved to global scope
typedef void (^MenuItemClickedBlock)(nativeapi::MenuItemId item_id, const std::string& item_text);
typedef void (^MenuOpenedBlock)(nativeapi::MenuId menu_id);
typedef void (^MenuClosedBlock)(nativeapi::MenuId menu_id);

@interface NSMenuItemTarget : NSObject
@property(nonatomic, copy) MenuItemClickedBlock clickedBlock;
- (void)menuItemClicked:(id)sender;
@end

@interface NSMenuDelegateImpl : NSObject <NSMenuDelegate>
@property(nonatomic, copy) MenuOpenedBlock openedBlock;
@property(nonatomic, copy) MenuClosedBlock closedBlock;
@end

namespace nativeapi {

// Global ID generators
static std::atomic<MenuItemId> g_next_menu_item_id{1};
static std::atomic<MenuId> g_next_menu_id{1};

// Removed global registries; events are dispatched via direct back-pointers

// Helper function to convert KeyboardAccelerator to NSString and modifier mask
std::pair<NSString*, NSUInteger> ConvertAccelerator(const KeyboardAccelerator& accelerator) {
  NSString* keyEquivalent = @"";
  NSUInteger modifierMask = 0;

  // Convert key
  if (!accelerator.key.empty()) {
    if (accelerator.key.length() == 1) {
      // Single character key
      char c = std::tolower(accelerator.key[0]);
      keyEquivalent = [NSString stringWithFormat:@"%c", c];
    } else {
      // Special keys
      std::string key = accelerator.key;
      if (key == "F1")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF1FunctionKey];
      else if (key == "F2")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF2FunctionKey];
      else if (key == "F3")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF3FunctionKey];
      else if (key == "F4")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF4FunctionKey];
      else if (key == "F5")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF5FunctionKey];
      else if (key == "F6")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF6FunctionKey];
      else if (key == "F7")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF7FunctionKey];
      else if (key == "F8")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF8FunctionKey];
      else if (key == "F9")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF9FunctionKey];
      else if (key == "F10")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF10FunctionKey];
      else if (key == "F11")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF11FunctionKey];
      else if (key == "F12")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF12FunctionKey];
      else if (key == "Enter" || key == "Return")
        keyEquivalent = @"\r";
      else if (key == "Tab")
        keyEquivalent = @"\t";
      else if (key == "Space")
        keyEquivalent = @" ";
      else if (key == "Escape")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)0x1B];
      else if (key == "Delete" || key == "Backspace")
        keyEquivalent = @"\b";
      else if (key == "ArrowUp")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSUpArrowFunctionKey];
      else if (key == "ArrowDown")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSDownArrowFunctionKey];
      else if (key == "ArrowLeft")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSLeftArrowFunctionKey];
      else if (key == "ArrowRight")
        keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSRightArrowFunctionKey];
    }
  }

  // Convert modifiers
  if (accelerator.modifiers & KeyboardAccelerator::Ctrl) {
    modifierMask |= NSEventModifierFlagControl;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Alt) {
    modifierMask |= NSEventModifierFlagOption;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Shift) {
    modifierMask |= NSEventModifierFlagShift;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Meta) {
    modifierMask |= NSEventModifierFlagCommand;
  }

  return std::make_pair(keyEquivalent, modifierMask);
}

}  // namespace nativeapi

// Implementation of NSMenuItemTarget - moved to global scope
@implementation NSMenuItemTarget
- (void)menuItemClicked:(id)sender {
  @try {
    NSMenuItem* menuItem = (NSMenuItem*)sender;
    if (!menuItem)
      return;

    // Call the block if it exists
    if (_clickedBlock) {
      NSString* title = [menuItem title];
      std::string itemText = title ? [title UTF8String] : "";

      // Get the MenuItemId from the menu item's associated object
      NSNumber* itemIdObj = objc_getAssociatedObject(menuItem, kMenuItemIdKey);
      if (itemIdObj) {
        nativeapi::MenuItemId itemId = [itemIdObj longValue];
        _clickedBlock(itemId, itemText);
      }
    }
  } @catch (NSException* exception) {
    // Log the exception but don't crash
    NSLog(@"Exception in menuItemClicked: %@", [exception reason]);
  }
}

@end

// Implementation of NSMenuDelegateImpl - moved to global scope
@implementation NSMenuDelegateImpl
- (void)menuWillOpen:(NSMenu*)menu {
  @try {
    if (!menu)
      return;

    if (_openedBlock) {
      // Get the MenuId from the menu's associated object
      NSNumber* menuIdObj = objc_getAssociatedObject(menu, kMenuIdKey);
      if (menuIdObj) {
        nativeapi::MenuId menuId = [menuIdObj longValue];
        _openedBlock(menuId);
      }
    }
  } @catch (NSException* exception) {
    // Log the exception but don't crash
    NSLog(@"Exception in menuWillOpen: %@", [exception reason]);
  }
}

- (void)menuDidClose:(NSMenu*)menu {
  @try {
    if (!menu)
      return;

    if (_closedBlock) {
      // Get the MenuId from the menu's associated object
      NSNumber* menuIdObj = objc_getAssociatedObject(menu, kMenuIdKey);
      if (menuIdObj) {
        nativeapi::MenuId menuId = [menuIdObj longValue];
        _closedBlock(menuId);
      }
    }
  } @catch (NSException* exception) {
    // Log the exception but don't crash
    NSLog(@"Exception in menuDidClose: %@", [exception reason]);
  }
}
@end

namespace nativeapi {

// MenuItem::Impl implementation
class MenuItem::Impl {
 public:
  NSMenuItem* ns_menu_item_;
  NSMenuItemTarget* ns_menu_item_target_;
  MenuItemType type_;
  std::optional<std::string> text_;
  std::optional<std::string> icon_;
  std::optional<std::string> tooltip_;
  KeyboardAccelerator accelerator_;
  bool has_accelerator_;
  bool enabled_;
  bool visible_;
  MenuItemState state_;
  int radio_group_;
  std::shared_ptr<Menu> submenu_;
  size_t submenu_opened_listener_id_;
  size_t submenu_closed_listener_id_;
  MenuItemId menu_item_id_;

  Impl(NSMenuItem* menu_item, MenuItemType type)
      : ns_menu_item_(menu_item),
        ns_menu_item_target_([[NSMenuItemTarget alloc] init]),
        type_(type),
        accelerator_("", KeyboardAccelerator::None),
        has_accelerator_(false),
        enabled_(true),
        visible_(true),
        state_(MenuItemState::Unchecked),
        radio_group_(-1),
        submenu_opened_listener_id_(0),
        submenu_closed_listener_id_(0),
        menu_item_id_(-1) {
    [ns_menu_item_ setTarget:ns_menu_item_target_];
    [ns_menu_item_ setAction:@selector(menuItemClicked:)];
  }

  ~Impl() {
    // First, remove submenu listeners before cleaning up submenu reference
    if (submenu_ && submenu_opened_listener_id_ != 0) {
      submenu_->RemoveListener(submenu_opened_listener_id_);
      submenu_opened_listener_id_ = 0;
    }
    if (submenu_ && submenu_closed_listener_id_ != 0) {
      submenu_->RemoveListener(submenu_closed_listener_id_);
      submenu_closed_listener_id_ = 0;
    }

    // Then clean up submenu reference
    if (submenu_) {
      submenu_.reset();
    }

    if (ns_menu_item_target_) {
      // Clean up blocks first
      ns_menu_item_target_.clickedBlock = nil;

      // Remove target and action to prevent callbacks after destruction
      [ns_menu_item_ setTarget:nil];
      [ns_menu_item_ setAction:nil];
      ns_menu_item_target_ = nil;
    }
  }
};

// MenuItem implementation
MenuItem::MenuItem(const std::string& text, MenuItemType type) : id(g_next_menu_item_id++) {
  NSMenuItem* nsItem = nullptr;

  switch (type) {
    case MenuItemType::Separator:
      nsItem = [NSMenuItem separatorItem];
      break;
    case MenuItemType::Normal:
    case MenuItemType::Checkbox:
    case MenuItemType::Radio:
    case MenuItemType::Submenu:
    default:
      nsItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:text.c_str()]
                                          action:nil
                                   keyEquivalent:@""];
      break;
  }

  pimpl_ = std::make_unique<Impl>(nsItem, type);
  pimpl_->menu_item_id_ = id;
  objc_setAssociatedObject(nsItem, kMenuItemIdKey, [NSNumber numberWithLong:id],
                           OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  pimpl_->text_ = text.empty() ? std::nullopt : std::optional<std::string>(text);

  // 设置默认的 Block 处理器，直接发送事件
  pimpl_->ns_menu_item_target_.clickedBlock = ^(MenuItemId item_id, const std::string& item_text) {
    try {
      EmitSync<MenuItemClickedEvent>(item_id, item_text);
    } catch (...) {
      // Protect against event emission exceptions
    }
  };
}

MenuItem::MenuItem(void* native_item)
    : id(g_next_menu_item_id++),
      pimpl_(std::make_unique<Impl>((__bridge NSMenuItem*)native_item, MenuItemType::Normal)) {
  NSMenuItem* nsItem = (__bridge NSMenuItem*)native_item;
  pimpl_->menu_item_id_ = id;
  objc_setAssociatedObject(nsItem, kMenuItemIdKey, [NSNumber numberWithLong:id],
                           OBJC_ASSOCIATION_RETAIN_NONATOMIC);

  // 设置默认的 Block 处理器，直接发送事件
  pimpl_->ns_menu_item_target_.clickedBlock = ^(MenuItemId item_id, const std::string& item_text) {
    try {
      EmitSync<MenuItemClickedEvent>(item_id, item_text);
    } catch (...) {
      // Protect against event emission exceptions
    }
  };
}

MenuItem::~MenuItem() {
  // No special cleanup needed since we're not storing C++ object references
}

MenuItemType MenuItem::GetType() const {
  return pimpl_->type_;
}

void MenuItem::SetLabel(const std::optional<std::string>& label) {
  pimpl_->text_ = label;
  if (label.has_value()) {
    [pimpl_->ns_menu_item_ setTitle:[NSString stringWithUTF8String:label->c_str()]];
  } else {
    [pimpl_->ns_menu_item_ setTitle:@""];
  }
}

std::optional<std::string> MenuItem::GetLabel() const {
  return pimpl_->text_;
}

void MenuItem::SetIcon(const std::optional<std::string>& icon) {
  pimpl_->icon_ = icon;

  NSImage* image = nil;

  if (icon.has_value()) {
    const std::string& iconStr = icon.value();

    // Check if the icon is a base64 string
    if (iconStr.find("data:image") != std::string::npos) {
      // Extract the base64 part
      size_t pos = iconStr.find("base64,");
      if (pos != std::string::npos) {
        std::string base64Icon = iconStr.substr(pos + 7);

        // Convert base64 to NSData
        NSString* base64String = [NSString stringWithUTF8String:base64Icon.c_str()];
        NSData* imageData = [[NSData alloc]
            initWithBase64EncodedString:base64String
                                options:NSDataBase64DecodingIgnoreUnknownCharacters];

        if (imageData) {
          image = [[NSImage alloc] initWithData:imageData];
        }
      }
    } else if (!iconStr.empty()) {
      // Try to load as file path first
      NSString* iconPath = [NSString stringWithUTF8String:iconStr.c_str()];
      image = [[NSImage alloc] initWithContentsOfFile:iconPath];

      // If that fails, try as named image
      if (!image) {
        image = [NSImage imageNamed:iconPath];
      }
    }
  }

  if (image) {
    [image setSize:NSMakeSize(16, 16)];  // Standard menu item icon size
    [image setTemplate:YES];
    [pimpl_->ns_menu_item_ setImage:image];
  } else {
    // Clear the image if no valid icon is provided
    [pimpl_->ns_menu_item_ setImage:nil];
  }
}

std::optional<std::string> MenuItem::GetIcon() const {
  return pimpl_->icon_;
}

void MenuItem::SetTooltip(const std::optional<std::string>& tooltip) {
  pimpl_->tooltip_ = tooltip;
  if (tooltip.has_value()) {
    [pimpl_->ns_menu_item_ setToolTip:[NSString stringWithUTF8String:tooltip->c_str()]];
  } else {
    [pimpl_->ns_menu_item_ setToolTip:nil];
  }
}

std::optional<std::string> MenuItem::GetTooltip() const {
  return pimpl_->tooltip_;
}

void MenuItem::SetAccelerator(const KeyboardAccelerator& accelerator) {
  pimpl_->accelerator_ = accelerator;
  pimpl_->has_accelerator_ = true;

  auto keyAndModifier = ConvertAccelerator(accelerator);
  [pimpl_->ns_menu_item_ setKeyEquivalent:keyAndModifier.first];
  [pimpl_->ns_menu_item_ setKeyEquivalentModifierMask:keyAndModifier.second];
}

KeyboardAccelerator MenuItem::GetAccelerator() const {
  if (pimpl_->has_accelerator_) {
    return pimpl_->accelerator_;
  }
  return KeyboardAccelerator("", KeyboardAccelerator::None);
}

void MenuItem::RemoveAccelerator() {
  pimpl_->has_accelerator_ = false;
  pimpl_->accelerator_ = KeyboardAccelerator("", KeyboardAccelerator::None);
  [pimpl_->ns_menu_item_ setKeyEquivalent:@""];
  [pimpl_->ns_menu_item_ setKeyEquivalentModifierMask:0];
}

void MenuItem::SetEnabled(bool enabled) {
  pimpl_->enabled_ = enabled;
  [pimpl_->ns_menu_item_ setEnabled:enabled];
}

bool MenuItem::IsEnabled() const {
  return pimpl_->enabled_;
}

void MenuItem::SetVisible(bool visible) {
  pimpl_->visible_ = visible;
  [pimpl_->ns_menu_item_ setHidden:!visible];
}

bool MenuItem::IsVisible() const {
  return pimpl_->visible_;
}

void MenuItem::SetState(MenuItemState state) {
  if (pimpl_->type_ == MenuItemType::Checkbox || pimpl_->type_ == MenuItemType::Radio) {
    // Radio buttons don't support Mixed state
    if (pimpl_->type_ == MenuItemType::Radio && state == MenuItemState::Mixed) {
      return;
    }

    pimpl_->state_ = state;

    // Set the appropriate NSControlStateValue
    NSControlStateValue nsState;
    switch (state) {
      case MenuItemState::Unchecked:
        nsState = NSControlStateValueOff;
        break;
      case MenuItemState::Checked:
        nsState = NSControlStateValueOn;
        break;
      case MenuItemState::Mixed:
        nsState = NSControlStateValueMixed;
        break;
    }
    [pimpl_->ns_menu_item_ setState:nsState];

    // Handle radio button group logic - uncheck siblings in the same NSMenu
    if (pimpl_->type_ == MenuItemType::Radio && state == MenuItemState::Checked &&
        pimpl_->radio_group_ >= 0) {
      NSMenu* parentMenu = [pimpl_->ns_menu_item_ menu];
      if (parentMenu) {
        for (NSMenuItem* sibling in [parentMenu itemArray]) {
          if (sibling == pimpl_->ns_menu_item_)
            continue;
          NSObject* targetObj = [sibling target];
          if ([targetObj isKindOfClass:[NSMenuItemTarget class]]) {
            // Get the MenuItemId from the associated object
            NSNumber* siblingIdObj = objc_getAssociatedObject(sibling, kMenuItemIdKey);
            if (siblingIdObj) {
              MenuItemId siblingId = [siblingIdObj longValue];
              // Find the corresponding MenuItem in the parent menu's items
              // This is a simplified approach - in practice, you might need to store
              // a reference to the parent menu or use a different strategy
              [sibling setState:NSControlStateValueOff];
            }
          }
        }
      }
    }
  }
}

MenuItemState MenuItem::GetState() const {
  return pimpl_->state_;
}

void MenuItem::SetRadioGroup(int group_id) {
  pimpl_->radio_group_ = group_id;
}

int MenuItem::GetRadioGroup() const {
  return pimpl_->radio_group_;
}

void MenuItem::SetSubmenu(std::shared_ptr<Menu> submenu) {
  try {
    pimpl_->submenu_ = submenu;
    if (submenu) {
      NSMenu* nsSubmenu = (__bridge NSMenu*)submenu->GetNativeObject();
      if (nsSubmenu) {
        [pimpl_->ns_menu_item_ setSubmenu:nsSubmenu];

        // Remove previous submenu listeners if they exist
        if (pimpl_->submenu_opened_listener_id_ != 0) {
          submenu->RemoveListener(pimpl_->submenu_opened_listener_id_);
          pimpl_->submenu_opened_listener_id_ = 0;
        }
        if (pimpl_->submenu_closed_listener_id_ != 0) {
          submenu->RemoveListener(pimpl_->submenu_closed_listener_id_);
          pimpl_->submenu_closed_listener_id_ = 0;
        }

        // Add event listeners to forward submenu events
        MenuItemId menu_item_id = id;
        MenuItem* self = this;
        pimpl_->submenu_opened_listener_id_ = submenu->AddListener<MenuOpenedEvent>(
            [self, menu_item_id](const MenuOpenedEvent& event) {
              try {
                self->EmitSync<MenuItemSubmenuOpenedEvent>(menu_item_id);
              } catch (...) {
                // Protect against event emission exceptions
              }
            });

        pimpl_->submenu_closed_listener_id_ = submenu->AddListener<MenuClosedEvent>(
            [self, menu_item_id](const MenuClosedEvent& event) {
              try {
                self->EmitSync<MenuItemSubmenuClosedEvent>(menu_item_id);
              } catch (...) {
                // Protect against event emission exceptions
              }
            });
      }
    } else {
      // Remove listeners when submenu is cleared
      if (pimpl_->submenu_ && pimpl_->submenu_opened_listener_id_ != 0) {
        pimpl_->submenu_->RemoveListener(pimpl_->submenu_opened_listener_id_);
        pimpl_->submenu_opened_listener_id_ = 0;
      }
      if (pimpl_->submenu_ && pimpl_->submenu_closed_listener_id_ != 0) {
        pimpl_->submenu_->RemoveListener(pimpl_->submenu_closed_listener_id_);
        pimpl_->submenu_closed_listener_id_ = 0;
      }
      [pimpl_->ns_menu_item_ setSubmenu:nil];
    }
  } catch (...) {
    // Handle C++ exceptions
    NSLog(@"Exception in SetSubmenu");
  }
}

std::shared_ptr<Menu> MenuItem::GetSubmenu() const {
  return pimpl_->submenu_;
}

void MenuItem::RemoveSubmenu() {
  // Remove listeners before clearing submenu reference
  if (pimpl_->submenu_ && pimpl_->submenu_opened_listener_id_ != 0) {
    pimpl_->submenu_->RemoveListener(pimpl_->submenu_opened_listener_id_);
    pimpl_->submenu_opened_listener_id_ = 0;
  }
  if (pimpl_->submenu_ && pimpl_->submenu_closed_listener_id_ != 0) {
    pimpl_->submenu_->RemoveListener(pimpl_->submenu_closed_listener_id_);
    pimpl_->submenu_closed_listener_id_ = 0;
  }

  pimpl_->submenu_.reset();
  [pimpl_->ns_menu_item_ setSubmenu:nil];
}

bool MenuItem::Trigger() {
  if (!pimpl_->enabled_)
    return false;

  // Call the block directly instead of going through target-action
  if (pimpl_->ns_menu_item_target_.clickedBlock) {
    std::string itemText = pimpl_->text_.value_or("");
    pimpl_->ns_menu_item_target_.clickedBlock(id, itemText);
  }
  return true;
}

void* MenuItem::GetNativeObjectInternal() const {
  return (__bridge void*)pimpl_->ns_menu_item_;
}

// Menu::Impl implementation
class Menu::Impl {
 public:
  NSMenu* ns_menu_;
  NSMenuDelegateImpl* delegate_;
  std::vector<std::shared_ptr<MenuItem>> items_;
  bool enabled_;
  bool visible_;

  Impl(NSMenu* menu)
      : ns_menu_(menu),
        delegate_([[NSMenuDelegateImpl alloc] init]),
        enabled_(true),
        visible_(false) {
    [ns_menu_ setDelegate:delegate_];
  }

  ~Impl() {
    // First, remove delegate to prevent callbacks during cleanup
    if (delegate_) {
      // Clean up blocks first
      delegate_.openedBlock = nil;
      delegate_.closedBlock = nil;

      [ns_menu_ setDelegate:nil];
      delegate_ = nil;
    }

    // Then clear all menu item references
    items_.clear();
  }
};

// Menu implementation
Menu::Menu() : id(g_next_menu_id++) {
  NSMenu* nsMenu = [[NSMenu alloc] init];
  pimpl_ = std::make_unique<Impl>(nsMenu);
  objc_setAssociatedObject(nsMenu, kMenuIdKey, [NSNumber numberWithLong:id],
                           OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  // 设置默认的 Block 处理器，直接发送事件
  pimpl_->delegate_.openedBlock = ^(MenuId menu_id) {
    try {
      EmitSync<MenuOpenedEvent>(menu_id);
    } catch (...) {
      // Protect against event emission exceptions
    }
  };

  pimpl_->delegate_.closedBlock = ^(MenuId menu_id) {
    try {
      EmitSync<MenuClosedEvent>(menu_id);
    } catch (...) {
      // Protect against event emission exceptions
    }
  };
}

Menu::Menu(void* native_menu)
    : id(g_next_menu_id++), pimpl_(std::make_unique<Impl>((__bridge NSMenu*)native_menu)) {
  NSMenu* nsMenu = (__bridge NSMenu*)native_menu;
  objc_setAssociatedObject(nsMenu, kMenuIdKey, [NSNumber numberWithLong:id],
                           OBJC_ASSOCIATION_RETAIN_NONATOMIC);

  // 设置默认的 Block 处理器，直接发送事件
  pimpl_->delegate_.openedBlock = ^(MenuId menu_id) {
    try {
      EmitSync<MenuOpenedEvent>(menu_id);
    } catch (...) {
      // Protect against event emission exceptions
    }
  };

  pimpl_->delegate_.closedBlock = ^(MenuId menu_id) {
    try {
      EmitSync<MenuClosedEvent>(menu_id);
    } catch (...) {
      // Protect against event emission exceptions
    }
  };
}

Menu::~Menu() {
  // No special cleanup needed since we're not storing C++ object references
}

void Menu::AddItem(std::shared_ptr<MenuItem> item) {
  if (!item)
    return;

  pimpl_->items_.push_back(item);
  [pimpl_->ns_menu_ addItem:(__bridge NSMenuItem*)item->GetNativeObject()];
}

void Menu::InsertItem(size_t index, std::shared_ptr<MenuItem> item) {
  if (!item)
    return;

  if (index >= pimpl_->items_.size()) {
    AddItem(item);
    return;
  }

  pimpl_->items_.insert(pimpl_->items_.begin() + index, item);
  [pimpl_->ns_menu_ insertItem:(__bridge NSMenuItem*)item->GetNativeObject() atIndex:index];
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
  if (!item)
    return false;

  auto it = std::find(pimpl_->items_.begin(), pimpl_->items_.end(), item);
  if (it != pimpl_->items_.end()) {
    [pimpl_->ns_menu_ removeItem:(__bridge NSMenuItem*)item->GetNativeObject()];
    pimpl_->items_.erase(it);
    return true;
  }
  return false;
}

bool Menu::RemoveItemById(MenuItemId item_id) {
  for (auto it = pimpl_->items_.begin(); it != pimpl_->items_.end(); ++it) {
    if ((*it)->id == item_id) {
      [pimpl_->ns_menu_ removeItem:(__bridge NSMenuItem*)(*it)->GetNativeObject()];
      pimpl_->items_.erase(it);
      return true;
    }
  }
  return false;
}

bool Menu::RemoveItemAt(size_t index) {
  if (index >= pimpl_->items_.size())
    return false;

  auto item = pimpl_->items_[index];
  [pimpl_->ns_menu_ removeItem:(__bridge NSMenuItem*)item->GetNativeObject()];
  pimpl_->items_.erase(pimpl_->items_.begin() + index);
  return true;
}

void Menu::Clear() {
  [pimpl_->ns_menu_ removeAllItems];
  pimpl_->items_.clear();
}

void Menu::AddSeparator() {
  auto separator = std::make_shared<MenuItem>("", MenuItemType::Separator);
  AddItem(separator);
}

void Menu::InsertSeparator(size_t index) {
  auto separator = std::make_shared<MenuItem>("", MenuItemType::Separator);
  InsertItem(index, separator);
}

size_t Menu::GetItemCount() const {
  return pimpl_->items_.size();
}

std::shared_ptr<MenuItem> Menu::GetItemAt(size_t index) const {
  if (index >= pimpl_->items_.size())
    return nullptr;
  return pimpl_->items_[index];
}

std::shared_ptr<MenuItem> Menu::GetItemById(MenuItemId item_id) const {
  for (const auto& item : pimpl_->items_) {
    if (item->id == item_id) {
      return item;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<MenuItem>> Menu::GetAllItems() const {
  return pimpl_->items_;
}

std::shared_ptr<MenuItem> Menu::FindItemByText(const std::string& text, bool case_sensitive) const {
  for (const auto& item : pimpl_->items_) {
    auto itemTextOpt = item->GetLabel();
    if (!itemTextOpt.has_value()) {
      continue;
    }

    const std::string& itemText = itemTextOpt.value();
    if (case_sensitive) {
      if (itemText == text)
        return item;
    } else {
      std::string lowerItemText = itemText;
      std::string lowerSearchText = text;
      std::transform(lowerItemText.begin(), lowerItemText.end(), lowerItemText.begin(), ::tolower);
      std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(),
                     ::tolower);
      if (lowerItemText == lowerSearchText)
        return item;
    }
  }
  return nullptr;
}

bool Menu::Open(double x, double y) {
  // Get the main window
  NSWindow* mainWindow = [[NSApplication sharedApplication] mainWindow];
  if (!mainWindow) {
    // Fallback to key window if main window is not available
    mainWindow = [[NSApplication sharedApplication] keyWindow];
  }

  if (!mainWindow) {
    // If still no window, use the old implementation
    NSPoint point = NSMakePoint(x, y);
    NSEvent* event = [NSEvent mouseEventWithType:NSEventTypeRightMouseDown
                                        location:point
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                     eventNumber:0
                                      clickCount:1
                                        pressure:1.0];

    pimpl_->visible_ = true;

    @autoreleasepool {
      NSView* dummyView = [[NSView alloc] init];
      [NSMenu popUpContextMenu:pimpl_->ns_menu_ withEvent:event forView:dummyView];
    }

    pimpl_->visible_ = false;
    return true;
  }

  NSView* contentView = [mainWindow contentView];
  if (!contentView) {
    return false;
  }

  // Convert coordinates if the content view is not flipped
  // In macOS, the default coordinate system has origin at bottom-left
  // If view is not flipped, we need to convert from top-left origin
  CGFloat finalY = y;
  if (![contentView isFlipped]) {
    CGFloat frameHeight = [contentView frame].size.height;
    finalY = frameHeight - y;
  }

  NSPoint point = NSMakePoint(x, finalY);

  pimpl_->visible_ = true;

  // Use dispatch to ensure menu popup happens on the main run loop
  dispatch_async(dispatch_get_main_queue(), ^{
    [pimpl_->ns_menu_ popUpMenuPositioningItem:nil atLocation:point inView:contentView];
  });

  return true;
}

bool Menu::Open() {
  NSPoint mouseLocation = [NSEvent mouseLocation];
  return Open(mouseLocation.x, mouseLocation.y);
}

bool Menu::Close() {
  if (pimpl_->visible_) {
    [pimpl_->ns_menu_ cancelTracking];
    pimpl_->visible_ = false;
    return true;
  }
  return false;
}

bool Menu::IsVisible() const {
  return pimpl_->visible_;
}

void Menu::SetEnabled(bool enabled) {
  pimpl_->enabled_ = enabled;
  // Enable/disable all items
  for (auto& item : pimpl_->items_) {
    item->SetEnabled(enabled);
  }
}

bool Menu::IsEnabled() const {
  return pimpl_->enabled_;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddItem(const std::string& text) {
  auto item = std::make_shared<MenuItem>(text, MenuItemType::Normal);
  AddItem(item);
  return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddItem(const std::string& text,
                                                 const std::optional<std::string>& icon) {
  auto item = std::make_shared<MenuItem>(text, MenuItemType::Normal);
  if (icon.has_value()) {
    item->SetIcon(icon);
  }
  AddItem(item);
  return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddSubmenu(const std::string& text,
                                                    std::shared_ptr<Menu> submenu) {
  auto item = std::make_shared<MenuItem>(text, MenuItemType::Submenu);
  item->SetSubmenu(submenu);
  AddItem(item);
  return item;
}

void* Menu::GetNativeObjectInternal() const {
  return (__bridge void*)pimpl_->ns_menu_;
}

void Menu::EmitOpenedEvent() {
  // This method is kept for compatibility but events are now handled through blocks
  // Call the block if it exists
  if (pimpl_->delegate_.openedBlock) {
    pimpl_->delegate_.openedBlock(id);
  }
}

void Menu::EmitClosedEvent() {
  // This method is kept for compatibility but events are now handled through blocks
  // Call the block if it exists
  if (pimpl_->delegate_.closedBlock) {
    pimpl_->delegate_.closedBlock(id);
  }
}
}  // namespace nativeapi
