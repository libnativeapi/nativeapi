#include <iostream>
#include <atomic>
#include <unordered_map>
#include <vector>
#include "../../menu.h"
#include "../../menu_event.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

// Note: This file assumes ARC (Automatic Reference Counting) is enabled
// for proper memory management of Objective-C objects.

// Forward declarations - moved to global scope
@interface MenuItemTarget : NSObject
@property (nonatomic, assign) nativeapi::MenuItemID itemId;
- (void)menuItemClicked:(id)sender;
@end

@interface MenuDelegate : NSObject <NSMenuDelegate>
@property (nonatomic, assign) nativeapi::MenuID menuId;
@end

namespace nativeapi {

// Global ID generators
static std::atomic<MenuItemID> g_next_menu_item_id{1};
static std::atomic<MenuID> g_next_menu_id{1};

// Global registry to map native objects to C++ objects for event emission
static std::unordered_map<MenuItemID, MenuItem*> g_menu_item_registry;
static std::unordered_map<MenuID, Menu*> g_menu_registry;

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
            if (key == "F1") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF1FunctionKey];
            else if (key == "F2") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF2FunctionKey];
            else if (key == "F3") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF3FunctionKey];
            else if (key == "F4") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF4FunctionKey];
            else if (key == "F5") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF5FunctionKey];
            else if (key == "F6") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF6FunctionKey];
            else if (key == "F7") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF7FunctionKey];
            else if (key == "F8") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF8FunctionKey];
            else if (key == "F9") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF9FunctionKey];
            else if (key == "F10") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF10FunctionKey];
            else if (key == "F11") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF11FunctionKey];
            else if (key == "F12") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSF12FunctionKey];
            else if (key == "Enter" || key == "Return") keyEquivalent = @"\r";
            else if (key == "Tab") keyEquivalent = @"\t";
            else if (key == "Space") keyEquivalent = @" ";
            else if (key == "Escape") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)0x1B];
            else if (key == "Delete" || key == "Backspace") keyEquivalent = @"\b";
            else if (key == "ArrowUp") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSUpArrowFunctionKey];
            else if (key == "ArrowDown") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSDownArrowFunctionKey];
            else if (key == "ArrowLeft") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSLeftArrowFunctionKey];
            else if (key == "ArrowRight") keyEquivalent = [NSString stringWithFormat:@"%C", (unichar)NSRightArrowFunctionKey];
        }
    }

    // Convert modifiers
    if (accelerator.modifiers & KeyboardAccelerator::Ctrl) {
        modifierMask |= NSEventModifierFlagCommand; // On macOS, Ctrl maps to Cmd
    }
    if (accelerator.modifiers & KeyboardAccelerator::Alt) {
        modifierMask |= NSEventModifierFlagOption;
    }
    if (accelerator.modifiers & KeyboardAccelerator::Shift) {
        modifierMask |= NSEventModifierFlagShift;
    }
    if (accelerator.modifiers & KeyboardAccelerator::Meta) {
        modifierMask |= NSEventModifierFlagControl; // On macOS, Meta maps to Ctrl
    }

    return std::make_pair(keyEquivalent, modifierMask);
}

}  // namespace nativeapi

// Implementation of MenuItemTarget - moved to global scope
@implementation MenuItemTarget
- (void)menuItemClicked:(id)sender {
    NSMenuItem* menuItem = (NSMenuItem*)sender;

    // Find the MenuItem object in registry
    auto menuItemIt = nativeapi::g_menu_item_registry.find(_itemId);
    if (menuItemIt == nativeapi::g_menu_item_registry.end()) {
        return; // Item not found in registry
    }

    nativeapi::MenuItem* menuItemPtr = menuItemIt->second;
    nativeapi::MenuItemType itemType = menuItemPtr->GetType();

    // Don't automatically handle state changes - let user code control the state
    // This prevents double-toggling when user event handlers also call SetChecked

    // Emit click event for all types
    menuItemPtr->EmitSelectedEvent([[menuItem title] UTF8String]);
}
@end

// Implementation of MenuDelegate - moved to global scope
@implementation MenuDelegate
- (void)menuWillOpen:(NSMenu *)menu {
    auto menuIt = nativeapi::g_menu_registry.find(_menuId);
    if (menuIt != nativeapi::g_menu_registry.end()) {
        nativeapi::Menu* menuPtr = menuIt->second;
        menuPtr->EmitOpenedEvent();
    }
}

- (void)menuDidClose:(NSMenu *)menu {
    auto menuIt = nativeapi::g_menu_registry.find(_menuId);
    if (menuIt != nativeapi::g_menu_registry.end()) {
        nativeapi::Menu* menuPtr = menuIt->second;
        menuPtr->EmitClosedEvent();
    }
}
@end

namespace nativeapi {

// MenuItem::Impl implementation
class MenuItem::Impl {
 public:
    NSMenuItem* ns_menu_item_;
    MenuItemTarget* target_;
    MenuItemType type_;
    std::string text_;
    std::string icon_;
    std::string tooltip_;
    KeyboardAccelerator accelerator_;
    bool has_accelerator_;
    bool enabled_;
    bool visible_;
    MenuItemState state_;
    int radio_group_;
    std::shared_ptr<Menu> submenu_;

    Impl(NSMenuItem* menu_item, MenuItemType type)
        : ns_menu_item_(menu_item)
        , target_([[MenuItemTarget alloc] init])
        , type_(type)
        , accelerator_("", KeyboardAccelerator::None)
        , has_accelerator_(false)
        , enabled_(true)
        , visible_(true)
        , state_(MenuItemState::Unchecked)
        , radio_group_(-1) {

        [ns_menu_item_ setTarget:target_];
        [ns_menu_item_ setAction:@selector(menuItemClicked:)];
    }

    ~Impl() {
        // First, clean up submenu reference
        if (submenu_) {
            submenu_.reset();
        }
        
        if (target_) {
            // Remove target and action to prevent callbacks after destruction
            [ns_menu_item_ setTarget:nil];
            [ns_menu_item_ setAction:nil];
            target_ = nil;
        }
    }
};

// MenuItem implementation
std::shared_ptr<MenuItem> MenuItem::Create(const std::string& text, MenuItemType type) {
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

    auto item = std::shared_ptr<MenuItem>(new MenuItem(text, type));
    item->pimpl_ = std::make_unique<Impl>(nsItem, type);
    item->id = g_next_menu_item_id++;
    item->pimpl_->target_.itemId = item->id;
    item->pimpl_->text_ = text;

    // Register the MenuItem for event emission
    g_menu_item_registry[item->id] = item.get();

    return item;
}

std::shared_ptr<MenuItem> MenuItem::CreateSeparator() {
    return Create("", MenuItemType::Separator);
}

MenuItem::MenuItem(void* native_item)
    : id(g_next_menu_item_id++)
    , pimpl_(std::make_unique<Impl>((__bridge NSMenuItem*)native_item, MenuItemType::Normal)) {
    pimpl_->target_.itemId = id;
    // Register the MenuItem for event emission
    g_menu_item_registry[id] = this;
}

MenuItem::MenuItem(const std::string& text, MenuItemType type)
    : id(0) { // Will be set in Create method
}

MenuItem::~MenuItem() {
    // Safely remove from global registry
    auto it = g_menu_item_registry.find(id);
    if (it != g_menu_item_registry.end()) {
        g_menu_item_registry.erase(it);
    }
}

MenuItemType MenuItem::GetType() const {
    return pimpl_->type_;
}

void MenuItem::SetLabel(const std::string& label) {
    pimpl_->text_ = label;
    [pimpl_->ns_menu_item_ setTitle:[NSString stringWithUTF8String:label.c_str()]];
}

std::string MenuItem::GetLabel() const {
    return pimpl_->text_;
}

void MenuItem::SetIcon(const std::string& icon) {
    pimpl_->icon_ = icon;

    NSImage* image = nil;

    // Check if the icon is a base64 string
    if (icon.find("data:image") != std::string::npos) {
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
                image = [[NSImage alloc] initWithData:imageData];
            }
        }
    } else if (!icon.empty()) {
        // Try to load as file path first
        NSString* iconPath = [NSString stringWithUTF8String:icon.c_str()];
        image = [[NSImage alloc] initWithContentsOfFile:iconPath];

        // If that fails, try as named image
        if (!image) {
            image = [NSImage imageNamed:iconPath];
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

std::string MenuItem::GetIcon() const {
    return pimpl_->icon_;
}

void MenuItem::SetTooltip(const std::string& tooltip) {
    pimpl_->tooltip_ = tooltip;
    [pimpl_->ns_menu_item_ setToolTip:[NSString stringWithUTF8String:tooltip.c_str()]];
}

std::string MenuItem::GetTooltip() const {
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

        // Handle radio button group logic - uncheck others in the same group
        if (pimpl_->type_ == MenuItemType::Radio && state == MenuItemState::Checked && pimpl_->radio_group_ >= 0) {
            for (auto& pair : g_menu_item_registry) {
                MenuItem* otherItem = pair.second;
                if (otherItem != this &&
                    otherItem->GetType() == MenuItemType::Radio &&
                    otherItem->GetRadioGroup() == pimpl_->radio_group_) {
                    otherItem->pimpl_->state_ = MenuItemState::Unchecked;
                    [otherItem->pimpl_->ns_menu_item_ setState:NSControlStateValueOff];
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
    pimpl_->submenu_ = submenu;
    if (submenu) {
        [pimpl_->ns_menu_item_ setSubmenu:(__bridge NSMenu*)submenu->GetNativeObject()];
    } else {
        [pimpl_->ns_menu_item_ setSubmenu:nil];
    }
}

std::shared_ptr<Menu> MenuItem::GetSubmenu() const {
    return pimpl_->submenu_;
}

void MenuItem::RemoveSubmenu() {
    pimpl_->submenu_.reset();
    [pimpl_->ns_menu_item_ setSubmenu:nil];
}


bool MenuItem::Trigger() {
    if (!pimpl_->enabled_) return false;

    [pimpl_->target_ menuItemClicked:pimpl_->ns_menu_item_];
    return true;
}

void* MenuItem::GetNativeObject() const {
    return (__bridge void*)pimpl_->ns_menu_item_;
}

void MenuItem::EmitSelectedEvent(const std::string& item_text) {
    EmitSync<MenuItemClickedEvent>(id, item_text);
}

void MenuItem::EmitStateChangedEvent(bool checked) {
    // This method is kept for compatibility but doesn't emit events
    // State change events are handled through the regular click event
}

// Menu::Impl implementation
class Menu::Impl {
 public:
    NSMenu* ns_menu_;
    MenuDelegate* delegate_;
    std::vector<std::shared_ptr<MenuItem>> items_;
    bool enabled_;
    bool visible_;

    Impl(NSMenu* menu)
        : ns_menu_(menu)
        , delegate_([[MenuDelegate alloc] init])
        , enabled_(true)
        , visible_(false) {
        [ns_menu_ setDelegate:delegate_];
    }

    ~Impl() {
        // First, clear all menu item references
        items_.clear();
        
        if (delegate_) {
            // Remove delegate to prevent callbacks after destruction
            [ns_menu_ setDelegate:nil];
            delegate_ = nil;
        }
    }
};

// Menu implementation
std::shared_ptr<Menu> Menu::Create() {
    NSMenu* nsMenu = [[NSMenu alloc] init];
    auto menu = std::shared_ptr<Menu>(new Menu());
    menu->pimpl_ = std::make_unique<Impl>(nsMenu);
    menu->id = g_next_menu_id++;
    menu->pimpl_->delegate_.menuId = menu->id;

    // Register the Menu for event emission
    g_menu_registry[menu->id] = menu.get();

    return menu;
}

Menu::Menu(void* native_menu)
    : id(g_next_menu_id++)
    , pimpl_(std::make_unique<Impl>((__bridge NSMenu*)native_menu)) {
    pimpl_->delegate_.menuId = id;
    // Register the Menu for event emission
    g_menu_registry[id] = this;
}

Menu::Menu()
    : id(0) { // Will be set in Create method
}

Menu::~Menu() {
    // Safely remove from global registry
    auto it = g_menu_registry.find(id);
    if (it != g_menu_registry.end()) {
        g_menu_registry.erase(it);
    }
}

void Menu::AddItem(std::shared_ptr<MenuItem> item) {
    if (!item) return;

    pimpl_->items_.push_back(item);
    [pimpl_->ns_menu_ addItem:(__bridge NSMenuItem*)item->GetNativeObject()];
}

void Menu::InsertItem(size_t index, std::shared_ptr<MenuItem> item) {
    if (!item) return;

    if (index >= pimpl_->items_.size()) {
        AddItem(item);
        return;
    }

    pimpl_->items_.insert(pimpl_->items_.begin() + index, item);
    [pimpl_->ns_menu_ insertItem:(__bridge NSMenuItem*)item->GetNativeObject() atIndex:index];
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
    if (!item) return false;

    auto it = std::find(pimpl_->items_.begin(), pimpl_->items_.end(), item);
    if (it != pimpl_->items_.end()) {
        [pimpl_->ns_menu_ removeItem:(__bridge NSMenuItem*)item->GetNativeObject()];
        pimpl_->items_.erase(it);
        return true;
    }
    return false;
}

bool Menu::RemoveItemById(MenuItemID item_id) {
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
    if (index >= pimpl_->items_.size()) return false;

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
    auto separator = MenuItem::CreateSeparator();
    AddItem(separator);
}

void Menu::InsertSeparator(size_t index) {
    auto separator = MenuItem::CreateSeparator();
    InsertItem(index, separator);
}

size_t Menu::GetItemCount() const {
    return pimpl_->items_.size();
}

std::shared_ptr<MenuItem> Menu::GetItemAt(size_t index) const {
    if (index >= pimpl_->items_.size()) return nullptr;
    return pimpl_->items_[index];
}

std::shared_ptr<MenuItem> Menu::GetItemById(MenuItemID item_id) const {
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
        std::string itemText = item->GetLabel();
        if (case_sensitive) {
            if (itemText == text) return item;
        } else {
            std::string lowerItemText = itemText;
            std::string lowerSearchText = text;
            std::transform(lowerItemText.begin(), lowerItemText.end(), lowerItemText.begin(), ::tolower);
            std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(), ::tolower);
            if (lowerItemText == lowerSearchText) return item;
        }
    }
    return nullptr;
}

bool Menu::ShowAsContextMenu(double x, double y) {
    NSPoint point = NSMakePoint(x, y);

    // Convert screen coordinates to window coordinates if needed
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
        // Create a dummy view to avoid the nil warning
        NSView* dummyView = [[NSView alloc] init];
        [NSMenu popUpContextMenu:pimpl_->ns_menu_ withEvent:event forView:dummyView];
    }

    pimpl_->visible_ = false;

    return true;
}

bool Menu::ShowAsContextMenu() {
    NSPoint mouseLocation = [NSEvent mouseLocation];
    return ShowAsContextMenu(mouseLocation.x, mouseLocation.y);
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
    auto item = MenuItem::Create(text, MenuItemType::Normal);
    AddItem(item);
    return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddItem(const std::string& text, const std::string& icon) {
    auto item = MenuItem::Create(text, MenuItemType::Normal);
    item->SetIcon(icon);
    AddItem(item);
    return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddSubmenu(const std::string& text, std::shared_ptr<Menu> submenu) {
    auto item = MenuItem::Create(text, MenuItemType::Submenu);
    item->SetSubmenu(submenu);
    AddItem(item);
    return item;
}

void* Menu::GetNativeObject() const {
    return (__bridge void*)pimpl_->ns_menu_;
}

void Menu::EmitOpenedEvent() {
    EmitSync<MenuOpenedEvent>(id);
}

void Menu::EmitClosedEvent() {
    EmitSync<MenuClosedEvent>(id);
}
}
