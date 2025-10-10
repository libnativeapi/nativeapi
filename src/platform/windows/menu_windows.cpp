#include <windows.h>
#include <algorithm>
#include <atomic>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "../../menu.h"
#include "../../menu_event.h"

namespace nativeapi {

// Global ID generators
static std::atomic<MenuItemID> g_next_menu_item_id{1};
static std::atomic<MenuID> g_next_menu_id{1};

// Global registry to map native objects to C++ objects for event emission
static std::unordered_map<MenuItemID, MenuItem*> g_menu_item_registry;
static std::unordered_map<MenuID, Menu*> g_menu_registry;

// Helper function to convert KeyboardAccelerator to Windows accelerator
std::pair<UINT, UINT> ConvertAccelerator(
    const KeyboardAccelerator& accelerator) {
  UINT key = 0;
  UINT modifiers = 0;

  // Convert key
  if (!accelerator.key.empty()) {
    if (accelerator.key.length() == 1) {
      // Single character key
      char c = std::toupper(accelerator.key[0]);
      key = static_cast<UINT>(c);
    } else {
      // Special keys
      std::string keyStr = accelerator.key;
      if (keyStr == "F1")
        key = VK_F1;
      else if (keyStr == "F2")
        key = VK_F2;
      else if (keyStr == "F3")
        key = VK_F3;
      else if (keyStr == "F4")
        key = VK_F4;
      else if (keyStr == "F5")
        key = VK_F5;
      else if (keyStr == "F6")
        key = VK_F6;
      else if (keyStr == "F7")
        key = VK_F7;
      else if (keyStr == "F8")
        key = VK_F8;
      else if (keyStr == "F9")
        key = VK_F9;
      else if (keyStr == "F10")
        key = VK_F10;
      else if (keyStr == "F11")
        key = VK_F11;
      else if (keyStr == "F12")
        key = VK_F12;
      else if (keyStr == "Enter" || keyStr == "Return")
        key = VK_RETURN;
      else if (keyStr == "Tab")
        key = VK_TAB;
      else if (keyStr == "Space")
        key = VK_SPACE;
      else if (keyStr == "Escape")
        key = VK_ESCAPE;
      else if (keyStr == "Delete" || keyStr == "Backspace")
        key = VK_BACK;
      else if (keyStr == "ArrowUp")
        key = VK_UP;
      else if (keyStr == "ArrowDown")
        key = VK_DOWN;
      else if (keyStr == "ArrowLeft")
        key = VK_LEFT;
      else if (keyStr == "ArrowRight")
        key = VK_RIGHT;
    }
  }

  // Convert modifiers
  if (accelerator.modifiers & KeyboardAccelerator::Ctrl) {
    modifiers |= FCONTROL;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Alt) {
    modifiers |= FALT;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Shift) {
    modifiers |= FSHIFT;
  }
  // Note: Windows doesn't have a direct equivalent for Meta key in accelerators

  return std::make_pair(key, modifiers);
}

}  // namespace nativeapi

namespace nativeapi {

// MenuItem::Impl implementation
class MenuItem::Impl {
 public:
  HMENU parent_menu_;
  UINT menu_item_id_;
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

  Impl(HMENU parent_menu, UINT menu_item_id, MenuItemType type)
      : parent_menu_(parent_menu),
        menu_item_id_(menu_item_id),
        type_(type),
        accelerator_("", KeyboardAccelerator::None),
        has_accelerator_(false),
        enabled_(true),
        visible_(true),
        state_(MenuItemState::Unchecked),
        radio_group_(-1) {}

  ~Impl() {
    // Windows menu items are automatically cleaned up when the menu is
    // destroyed
  }
};

// MenuItem implementation
std::shared_ptr<MenuItem> MenuItem::Create(const std::string& text,
                                           MenuItemType type) {
  auto item = std::shared_ptr<MenuItem>(new MenuItem(text, type));
  item->pimpl_ = std::make_unique<Impl>(nullptr, 0, type);
  item->id = g_next_menu_item_id++;
  item->pimpl_->text_ = text;

  // Register the MenuItem for event emission
  g_menu_item_registry[item->id] = item.get();

  return item;
}

std::shared_ptr<MenuItem> MenuItem::CreateSeparator() {
  return Create("", MenuItemType::Separator);
}

MenuItem::MenuItem(void* native_item)
    : id(g_next_menu_item_id++),
      pimpl_(std::make_unique<Impl>(nullptr, 0, MenuItemType::Normal)) {
  // Register the MenuItem for event emission
  g_menu_item_registry[id] = this;
}

MenuItem::MenuItem(const std::string& text, MenuItemType type)
    : id(0) {  // Will be set in Create method
}

MenuItem::~MenuItem() {
  // Unregister from event registry
  g_menu_item_registry.erase(id);
}

MenuItemType MenuItem::GetType() const {
  return pimpl_->type_;
}

void MenuItem::SetLabel(const std::string& label) {
  pimpl_->text_ = label;
  if (pimpl_->parent_menu_) {
    MENUITEMINFO mii = {};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = const_cast<LPSTR>(label.c_str());
    SetMenuItemInfo(pimpl_->parent_menu_, pimpl_->menu_item_id_, FALSE, &mii);
  }
}

std::string MenuItem::GetLabel() const {
  return pimpl_->text_;
}

void MenuItem::SetIcon(const std::string& icon) {
  pimpl_->icon_ = icon;
  // Windows menu icons would require HBITMAP handling
  // This is a placeholder implementation
}

std::string MenuItem::GetIcon() const {
  return pimpl_->icon_;
}

void MenuItem::SetTooltip(const std::string& tooltip) {
  pimpl_->tooltip_ = tooltip;
  // Windows doesn't have built-in tooltip support for menu items
  // This would require custom implementation
}

std::string MenuItem::GetTooltip() const {
  return pimpl_->tooltip_;
}

void MenuItem::SetAccelerator(const KeyboardAccelerator& accelerator) {
  pimpl_->accelerator_ = accelerator;
  pimpl_->has_accelerator_ = true;
  // Windows accelerators would be handled through accelerator tables
  // This is a placeholder implementation
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
}

void MenuItem::SetEnabled(bool enabled) {
  pimpl_->enabled_ = enabled;
  if (pimpl_->parent_menu_) {
    EnableMenuItem(pimpl_->parent_menu_, pimpl_->menu_item_id_,
                   enabled ? MF_ENABLED : MF_GRAYED);
  }
}

bool MenuItem::IsEnabled() const {
  return pimpl_->enabled_;
}

void MenuItem::SetVisible(bool visible) {
  pimpl_->visible_ = visible;
  // Windows doesn't have direct visibility control for menu items
  // Would need to remove/add items to simulate visibility
}

bool MenuItem::IsVisible() const {
  return pimpl_->visible_;
}

void MenuItem::SetState(MenuItemState state) {
  if (pimpl_->type_ == MenuItemType::Checkbox ||
      pimpl_->type_ == MenuItemType::Radio) {
    pimpl_->state_ = state;

    if (pimpl_->parent_menu_) {
      UINT checkState = MF_UNCHECKED;
      if (state == MenuItemState::Checked) {
        checkState = MF_CHECKED;
      }
      CheckMenuItem(pimpl_->parent_menu_, pimpl_->menu_item_id_, checkState);
    }

    // Handle radio button group logic
    if (pimpl_->type_ == MenuItemType::Radio &&
        state == MenuItemState::Checked && pimpl_->radio_group_ >= 0) {
      for (auto& pair : g_menu_item_registry) {
        MenuItem* otherItem = pair.second;
        if (otherItem != this && otherItem->GetType() == MenuItemType::Radio &&
            otherItem->GetRadioGroup() == pimpl_->radio_group_) {
          otherItem->pimpl_->state_ = MenuItemState::Unchecked;
          if (otherItem->pimpl_->parent_menu_) {
            CheckMenuItem(otherItem->pimpl_->parent_menu_,
                          otherItem->pimpl_->menu_item_id_, MF_UNCHECKED);
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
  pimpl_->submenu_ = submenu;
  if (pimpl_->parent_menu_ && submenu) {
    MENUITEMINFO mii = {};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = static_cast<HMENU>(submenu->GetNativeObject());
    SetMenuItemInfo(pimpl_->parent_menu_, pimpl_->menu_item_id_, FALSE, &mii);
  }
}

std::shared_ptr<Menu> MenuItem::GetSubmenu() const {
  return pimpl_->submenu_;
}

void MenuItem::RemoveSubmenu() {
  pimpl_->submenu_.reset();
  if (pimpl_->parent_menu_) {
    MENUITEMINFO mii = {};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = nullptr;
    SetMenuItemInfo(pimpl_->parent_menu_, pimpl_->menu_item_id_, FALSE, &mii);
  }
}

bool MenuItem::Trigger() {
  if (!pimpl_->enabled_)
    return false;

  EmitSelectedEvent(pimpl_->text_);
  return true;
}

void* MenuItem::GetNativeObjectInternal() const {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(pimpl_->menu_item_id_));
}

void MenuItem::EmitSelectedEvent(const std::string& item_text) {
  EmitSync<MenuItemClickedEvent>(id, item_text);
}

void MenuItem::EmitStateChangedEvent(bool checked) {
  // This method is kept for compatibility
}

// Menu::Impl implementation
class Menu::Impl {
 public:
  HMENU hmenu_;
  std::vector<std::shared_ptr<MenuItem>> items_;
  bool enabled_;
  bool visible_;

  Impl(HMENU menu) : hmenu_(menu), enabled_(true), visible_(false) {}

  ~Impl() {
    if (hmenu_) {
      DestroyMenu(hmenu_);
    }
  }
};

// Menu implementation
std::shared_ptr<Menu> Menu::Create() {
  HMENU hmenu = CreatePopupMenu();
  if (!hmenu) {
    return nullptr;
  }

  auto menu = std::shared_ptr<Menu>(new Menu());
  menu->pimpl_ = std::make_unique<Impl>(hmenu);
  menu->id = g_next_menu_id++;

  // Register the Menu for event emission
  g_menu_registry[menu->id] = menu.get();

  return menu;
}

Menu::Menu(void* native_menu)
    : id(g_next_menu_id++),
      pimpl_(std::make_unique<Impl>(static_cast<HMENU>(native_menu))) {
  // Register the Menu for event emission
  g_menu_registry[id] = this;
}

Menu::Menu() : id(0) {  // Will be set in Create method
}

Menu::~Menu() {
  // Unregister from event registry
  g_menu_registry.erase(id);
}

void Menu::AddItem(std::shared_ptr<MenuItem> item) {
  if (!item)
    return;

  pimpl_->items_.push_back(item);

  UINT flags = MF_STRING;
  if (item->GetType() == MenuItemType::Separator) {
    flags = MF_SEPARATOR;
  } else if (item->GetType() == MenuItemType::Checkbox) {
    flags |= MF_UNCHECKED;
  } else if (item->GetType() == MenuItemType::Radio) {
    flags |= MF_UNCHECKED;
  }

  UINT_PTR menuId = item->id;
  HMENU subMenu = nullptr;
  if (item->GetSubmenu()) {
    subMenu = static_cast<HMENU>(item->GetSubmenu()->GetNativeObject());
    flags |= MF_POPUP;
    menuId = reinterpret_cast<UINT_PTR>(subMenu);
  }

  AppendMenu(pimpl_->hmenu_, flags, menuId, item->GetLabel().c_str());

  // Update the item's impl with menu info
  item->pimpl_->parent_menu_ = pimpl_->hmenu_;
  item->pimpl_->menu_item_id_ = static_cast<UINT>(item->id);
}

void Menu::InsertItem(size_t index, std::shared_ptr<MenuItem> item) {
  if (!item)
    return;

  if (index >= pimpl_->items_.size()) {
    AddItem(item);
    return;
  }

  pimpl_->items_.insert(pimpl_->items_.begin() + index, item);

  UINT flags = MF_STRING | MF_BYPOSITION;
  if (item->GetType() == MenuItemType::Separator) {
    flags = MF_SEPARATOR | MF_BYPOSITION;
  }

  InsertMenu(pimpl_->hmenu_, static_cast<UINT>(index), flags, item->id,
             item->GetLabel().c_str());

  item->pimpl_->parent_menu_ = pimpl_->hmenu_;
  item->pimpl_->menu_item_id_ = static_cast<UINT>(item->id);
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
  if (!item)
    return false;

  auto it = std::find(pimpl_->items_.begin(), pimpl_->items_.end(), item);
  if (it != pimpl_->items_.end()) {
    RemoveMenu(pimpl_->hmenu_, item->id, MF_BYCOMMAND);
    pimpl_->items_.erase(it);
    return true;
  }
  return false;
}

bool Menu::RemoveItemById(MenuItemID item_id) {
  for (auto it = pimpl_->items_.begin(); it != pimpl_->items_.end(); ++it) {
    if ((*it)->id == item_id) {
      RemoveMenu(pimpl_->hmenu_, item_id, MF_BYCOMMAND);
      pimpl_->items_.erase(it);
      return true;
    }
  }
  return false;
}

bool Menu::RemoveItemAt(size_t index) {
  if (index >= pimpl_->items_.size())
    return false;

  RemoveMenu(pimpl_->hmenu_, static_cast<UINT>(index), MF_BYPOSITION);
  pimpl_->items_.erase(pimpl_->items_.begin() + index);
  return true;
}

void Menu::Clear() {
  while (GetMenuItemCount(pimpl_->hmenu_) > 0) {
    RemoveMenu(pimpl_->hmenu_, 0, MF_BYPOSITION);
  }
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
  if (index >= pimpl_->items_.size())
    return nullptr;
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

std::shared_ptr<MenuItem> Menu::FindItemByText(const std::string& text,
                                               bool case_sensitive) const {
  for (const auto& item : pimpl_->items_) {
    std::string itemText = item->GetLabel();
    if (case_sensitive) {
      if (itemText == text)
        return item;
    } else {
      std::string lowerItemText = itemText;
      std::string lowerSearchText = text;
      std::transform(lowerItemText.begin(), lowerItemText.end(),
                     lowerItemText.begin(), ::tolower);
      std::transform(lowerSearchText.begin(), lowerSearchText.end(),
                     lowerSearchText.begin(), ::tolower);
      if (lowerItemText == lowerSearchText)
        return item;
    }
  }
  return nullptr;
}

bool Menu::ShowAsContextMenu(double x, double y) {
  pimpl_->visible_ = true;

  POINT pt = {static_cast<int>(x), static_cast<int>(y)};
  HWND hwnd = GetActiveWindow();
  if (!hwnd) {
    hwnd = GetDesktopWindow();
  }

  // Show the context menu
  TrackPopupMenu(pimpl_->hmenu_, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);

  pimpl_->visible_ = false;
  return true;
}

bool Menu::ShowAsContextMenu() {
  POINT pt;
  GetCursorPos(&pt);
  return ShowAsContextMenu(pt.x, pt.y);
}

bool Menu::Close() {
  if (pimpl_->visible_) {
    // Windows automatically closes popup menus when user clicks elsewhere
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

std::shared_ptr<MenuItem> Menu::CreateAndAddItem(const std::string& text,
                                                 const std::string& icon) {
  auto item = MenuItem::Create(text, MenuItemType::Normal);
  item->SetIcon(icon);
  AddItem(item);
  return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddSubmenu(
    const std::string& text,
    std::shared_ptr<Menu> submenu) {
  auto item = MenuItem::Create(text, MenuItemType::Submenu);
  item->SetSubmenu(submenu);
  AddItem(item);
  return item;
}

void* Menu::GetNativeObjectInternal() const {
  return pimpl_->hmenu_;
}

void Menu::EmitOpenedEvent() {
  EmitSync<MenuOpenedEvent>(id);
}

void Menu::EmitClosedEvent() {
  EmitSync<MenuClosedEvent>(id);
}

}  // namespace nativeapi
