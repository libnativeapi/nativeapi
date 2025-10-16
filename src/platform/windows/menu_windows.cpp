#include <windows.h>
#include <memory>
#include <optional>
#include <vector>
#include "../../foundation/id_allocator.h"
#include "../../image.h"
#include "../../menu.h"
#include "../../menu_event.h"
#include "string_utils_windows.h"

namespace nativeapi {

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
      std::string key_str = accelerator.key;
      if (key_str == "F1")
        key = VK_F1;
      else if (key_str == "F2")
        key = VK_F2;
      else if (key_str == "F3")
        key = VK_F3;
      else if (key_str == "F4")
        key = VK_F4;
      else if (key_str == "F5")
        key = VK_F5;
      else if (key_str == "F6")
        key = VK_F6;
      else if (key_str == "F7")
        key = VK_F7;
      else if (key_str == "F8")
        key = VK_F8;
      else if (key_str == "F9")
        key = VK_F9;
      else if (key_str == "F10")
        key = VK_F10;
      else if (key_str == "F11")
        key = VK_F11;
      else if (key_str == "F12")
        key = VK_F12;
      else if (key_str == "Enter" || key_str == "Return")
        key = VK_RETURN;
      else if (key_str == "Tab")
        key = VK_TAB;
      else if (key_str == "Space")
        key = VK_SPACE;
      else if (key_str == "Escape")
        key = VK_ESCAPE;
      else if (key_str == "Delete" || key_str == "Backspace")
        key = VK_BACK;
      else if (key_str == "ArrowUp")
        key = VK_UP;
      else if (key_str == "ArrowDown")
        key = VK_DOWN;
      else if (key_str == "ArrowLeft")
        key = VK_LEFT;
      else if (key_str == "ArrowRight")
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
  std::optional<std::string> text_;
  std::shared_ptr<Image> image_;
  std::optional<std::string> tooltip_;
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

MenuItem::MenuItem(void* native_item)
    : id(IdAllocator::Allocate<MenuItem>()),
      pimpl_(std::make_unique<Impl>(nullptr, 0, MenuItemType::Normal)) {}

MenuItem::MenuItem(const std::string& text, MenuItemType type)
    : id(IdAllocator::Allocate<MenuItem>()),
      pimpl_(std::make_unique<Impl>(nullptr, 0, type)) {
  pimpl_->text_ = text;
}

MenuItem::~MenuItem() {}

MenuItemType MenuItem::GetType() const {
  return pimpl_->type_;
}

void MenuItem::SetLabel(const std::optional<std::string>& label) {
  pimpl_->text_ = label;
  if (pimpl_->parent_menu_) {
    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STRING;
    std::string label_str = label.has_value() ? *label : "";
    std::wstring w_label_str = StringToWString(label_str);
    mii.dwTypeData = const_cast<LPWSTR>(w_label_str.c_str());
    SetMenuItemInfoW(pimpl_->parent_menu_, pimpl_->menu_item_id_, FALSE, &mii);
  }
}

std::optional<std::string> MenuItem::GetLabel() const {
  return pimpl_->text_;
}

void MenuItem::SetIcon(std::shared_ptr<Image> image) {
  pimpl_->image_ = image;
  // Windows menu icons would require HBITMAP handling
  // This is a placeholder implementation
}

std::shared_ptr<Image> MenuItem::GetIcon() const {
  return pimpl_->image_;
}

void MenuItem::SetTooltip(const std::optional<std::string>& tooltip) {
  pimpl_->tooltip_ = tooltip;
  // Windows doesn't have built-in tooltip support for menu items
  // This would require custom implementation
}

std::optional<std::string> MenuItem::GetTooltip() const {
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
      UINT check_state = MF_UNCHECKED;
      if (state == MenuItemState::Checked) {
        check_state = MF_CHECKED;
      }
      CheckMenuItem(pimpl_->parent_menu_, pimpl_->menu_item_id_, check_state);
    }

    // Handle radio button group logic
    if (pimpl_->type_ == MenuItemType::Radio &&
        state == MenuItemState::Checked && pimpl_->radio_group_ >= 0) {
      // Note: Radio group logic would need to be implemented differently
      // without global registry. This could be done by maintaining group
      // information in the parent menu or through other means.
      // For now, this functionality is disabled.
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
    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = static_cast<HMENU>(submenu->GetNativeObject());
    SetMenuItemInfoW(pimpl_->parent_menu_, pimpl_->menu_item_id_, FALSE, &mii);
  }
}

std::shared_ptr<Menu> MenuItem::GetSubmenu() const {
  return pimpl_->submenu_;
}

void MenuItem::RemoveSubmenu() {
  pimpl_->submenu_.reset();
  if (pimpl_->parent_menu_) {
    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = nullptr;
    SetMenuItemInfoW(pimpl_->parent_menu_, pimpl_->menu_item_id_, FALSE, &mii);
  }
}

bool MenuItem::Trigger() {
  if (!pimpl_->enabled_)
    return false;

  try {
    std::string text = pimpl_->text_.has_value() ? pimpl_->text_.value() : "";
    EmitSync<MenuItemClickedEvent>(id, text);
  } catch (...) {
    // Protect against event emission exceptions
  }
  return true;
}

void* MenuItem::GetNativeObjectInternal() const {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(pimpl_->menu_item_id_));
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

Menu::Menu(void* native_menu)
    : id(IdAllocator::Allocate<Menu>()),
      pimpl_(std::make_unique<Impl>(static_cast<HMENU>(native_menu))) {}

Menu::Menu()
    : id(IdAllocator::Allocate<Menu>()),
      pimpl_(std::make_unique<Impl>(CreatePopupMenu())) {}

Menu::~Menu() {}

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

  UINT_PTR menu_id = item->id;
  HMENU sub_menu = nullptr;
  if (item->GetSubmenu()) {
    sub_menu = static_cast<HMENU>(item->GetSubmenu()->GetNativeObject());
    flags |= MF_POPUP;
    menu_id = reinterpret_cast<UINT_PTR>(sub_menu);
  }

  auto label_opt = item->GetLabel();
  std::string label_str = label_opt.has_value() ? *label_opt : "";
  std::wstring w_label_str = StringToWString(label_str);
  AppendMenuW(pimpl_->hmenu_, flags, menu_id, w_label_str.c_str());

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

  auto label_opt = item->GetLabel();
  std::string label_str = label_opt.has_value() ? *label_opt : "";
  std::wstring w_label_str = StringToWString(label_str);
  InsertMenuW(pimpl_->hmenu_, static_cast<UINT>(index), flags, item->id,
              w_label_str.c_str());

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

bool Menu::RemoveItemById(MenuItemId item_id) {
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

std::shared_ptr<MenuItem> Menu::FindItemByText(const std::string& text,
                                               bool case_sensitive) const {
  for (const auto& item : pimpl_->items_) {
    auto item_text_opt = item->GetLabel();
    if (!item_text_opt.has_value()) {
      continue;
    }

    const std::string& item_text = item_text_opt.value();
    if (case_sensitive) {
      if (item_text == text)
        return item;
    } else {
      std::string lower_item_text = item_text;
      std::string lower_search_text = text;
      std::transform(lower_item_text.begin(), lower_item_text.end(),
                     lower_item_text.begin(), ::tolower);
      std::transform(lower_search_text.begin(), lower_search_text.end(),
                     lower_search_text.begin(), ::tolower);
      if (lower_item_text == lower_search_text)
        return item;
    }
  }
  return nullptr;
}

bool Menu::Open(double x, double y) {
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

bool Menu::Open() {
  POINT pt;
  GetCursorPos(&pt);
  return Open(pt.x, pt.y);
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

void* Menu::GetNativeObjectInternal() const {
  return pimpl_->hmenu_;
}

}  // namespace nativeapi
