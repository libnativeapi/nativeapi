#include <windows.h>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>
#include "../../foundation/id_allocator.h"
#include "../../image.h"
#include "../../menu.h"
#include "../../menu_event.h"
#include "string_utils_windows.h"
#include "window_message_dispatcher.h"

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

// MenuItem::Impl implementation
class MenuItem::Impl {
 public:
  MenuItemId id_;
  HMENU parent_menu_;
  MenuItemType type_;
  std::optional<std::string> label_;
  std::shared_ptr<Image> image_;
  std::optional<std::string> tooltip_;
  KeyboardAccelerator accelerator_;
  bool has_accelerator_;
  bool enabled_;
  bool visible_;
  MenuItemState state_;
  int radio_group_;
  std::shared_ptr<Menu> submenu_;

  Impl(MenuItemId id, HMENU parent_menu, MenuItemType type)
      : id_(id),
        parent_menu_(parent_menu),
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
    : pimpl_(std::make_unique<Impl>(IdAllocator::Allocate<MenuItem>(),
                                    nullptr,
                                    MenuItemType::Normal)) {}

MenuItem::MenuItem(const std::string& label, MenuItemType type)
    : pimpl_(std::make_unique<Impl>(IdAllocator::Allocate<MenuItem>(),
                                    nullptr,
                                    type)) {
  pimpl_->label_ = label;
}

MenuItem::~MenuItem() {}

MenuItemId MenuItem::GetId() const {
  return pimpl_->id_;
}

MenuItemType MenuItem::GetType() const {
  return pimpl_->type_;
}

void MenuItem::SetLabel(const std::optional<std::string>& label) {
  pimpl_->label_ = label;
  if (pimpl_->parent_menu_) {
    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STRING;
    std::string label_str = label.has_value() ? *label : "";
    std::wstring w_label_str = StringToWString(label_str);
    mii.dwTypeData = const_cast<LPWSTR>(w_label_str.c_str());
    SetMenuItemInfoW(pimpl_->parent_menu_, pimpl_->id_, FALSE, &mii);
  }
}

std::optional<std::string> MenuItem::GetLabel() const {
  return pimpl_->label_;
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
    EnableMenuItem(pimpl_->parent_menu_, pimpl_->id_,
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
      CheckMenuItem(pimpl_->parent_menu_, pimpl_->id_, check_state);
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
    SetMenuItemInfoW(pimpl_->parent_menu_, pimpl_->id_, FALSE, &mii);
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
    SetMenuItemInfoW(pimpl_->parent_menu_, pimpl_->id_, FALSE, &mii);
  }
}

bool MenuItem::Trigger() {
  if (!pimpl_->enabled_)
    return false;

  try {
    Emit<MenuItemClickedEvent>(pimpl_->id_);
  } catch (...) {
    // Protect against event emission exceptions
  }
  return true;
}

void* MenuItem::GetNativeObjectInternal() const {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(pimpl_->id_));
}

// Menu::Impl implementation
class Menu::Impl {
 public:
  MenuId id_;
  HMENU hmenu_;
  std::vector<std::shared_ptr<MenuItem>> items_;
  bool enabled_;
  bool visible_;
  int window_proc_handle_id_;

  Impl(MenuId id, HMENU menu)
      : id_(id),
        hmenu_(menu),
        enabled_(true),
        visible_(false),
        window_proc_handle_id_(-1) {}

  ~Impl() {
    // Unregister window procedure handler
    if (window_proc_handle_id_ != -1) {
      WindowMessageDispatcher::GetInstance().UnregisterHandler(
          window_proc_handle_id_);
    }

    if (hmenu_) {
      DestroyMenu(hmenu_);
    }
  }

  // Handle window procedure delegate for menu commands and menu lifecycle events
  std::optional<LRESULT> HandleWindowProc(HWND hwnd,
                                          UINT message,
                                          WPARAM wparam,
                                          LPARAM lparam,
                                          Menu* menu) {
    // Handle menu lifecycle events
    if (message == WM_INITMENUPOPUP) {
      // wParam contains the HMENU handle of the popup menu being opened
      HMENU popup_menu = reinterpret_cast<HMENU>(wparam);
      
      if (popup_menu == hmenu_) {
        // This is our menu being opened
        visible_ = true;
        
        // Emit menu opened event
        try {
          menu->Emit<MenuOpenedEvent>(id_);
        } catch (...) {
          // Protect against event emission exceptions
        }
      }
    } else if (message == WM_UNINITMENUPOPUP) {
      // wParam contains the HMENU handle of the popup menu being closed
      HMENU popup_menu = reinterpret_cast<HMENU>(wparam);
      
      if (popup_menu == hmenu_) {
        // This is our menu being closed
        visible_ = false;
        
        // Emit menu closed event
        try {
          menu->Emit<MenuClosedEvent>(id_);
        } catch (...) {
          // Protect against event emission exceptions
        }
      }
    } else if (message == WM_COMMAND) {
      // For WM_COMMAND from menus, wparam contains the menu item ID
      // When using popup menus (TrackPopupMenu), the full 32-bit ID is
      // preserved in wparam, unlike menu bars which only use 16-bit IDs
      WORD hiword = HIWORD(wparam);
      WORD loword = LOWORD(wparam);

      std::cout << "Menu: WM_COMMAND - HIWORD=" << hiword
                << ", LOWORD=" << loword << std::endl;

      // For popup menus, lparam is NULL and wparam contains menu item ID
      if (lparam == 0) {
        // Reconstruct the full 32-bit menu item ID from wparam
        // wparam for menus contains the full ID we passed to AppendMenuW
        MenuItemId menu_item_id = static_cast<MenuItemId>(wparam);

        std::cout << "Menu: Looking for menu item with ID = " << menu_item_id
                  << " (0x" << std::hex << menu_item_id << std::dec << ")"
                  << std::endl;

        // Find the menu item by IdAllocator-generated ID
        for (const auto& item : items_) {
          if (item->pimpl_->id_ == menu_item_id) {
            std::cout << "Menu: Item clicked, ID = " << menu_item_id
                      << std::endl;
            // Call Trigger() to emit the event
            item->Trigger();
            return 0;
          }
        }

        std::cout << "Menu: No matching menu item found for ID " << menu_item_id
                  << std::endl;
      }
    }
    return std::nullopt;  // Let default window procedure handle it
  }
};

Menu::Menu(void* native_menu)
    : pimpl_(std::make_unique<Impl>(IdAllocator::Allocate<Menu>(),
                                    static_cast<HMENU>(native_menu))) {
  // Register window procedure handler for menu commands and events
  HWND host_window = WindowMessageDispatcher::GetInstance().GetHostWindow();
  if (host_window) {
    pimpl_->window_proc_handle_id_ =
        WindowMessageDispatcher::GetInstance().RegisterHandler(
            host_window,
            [this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
              return pimpl_->HandleWindowProc(hwnd, message, wparam, lparam, this);
            });
  }
}

Menu::Menu()
    : pimpl_(std::make_unique<Impl>(IdAllocator::Allocate<Menu>(),
                                    CreatePopupMenu())) {
  // Register window procedure handler for menu commands and events
  HWND host_window = WindowMessageDispatcher::GetInstance().GetHostWindow();
  if (host_window) {
    pimpl_->window_proc_handle_id_ =
        WindowMessageDispatcher::GetInstance().RegisterHandler(
            host_window,
            [this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
              return pimpl_->HandleWindowProc(hwnd, message, wparam, lparam, this);
            });
  }
}

Menu::~Menu() {}

MenuId Menu::GetId() const {
  return pimpl_->id_;
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

  UINT_PTR menu_id = item->GetId();
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
  InsertMenuW(pimpl_->hmenu_, static_cast<UINT>(index), flags, item->GetId(),
              w_label_str.c_str());

  item->pimpl_->parent_menu_ = pimpl_->hmenu_;
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
  if (!item)
    return false;

  auto it = std::find(pimpl_->items_.begin(), pimpl_->items_.end(), item);
  if (it != pimpl_->items_.end()) {
    RemoveMenu(pimpl_->hmenu_, item->GetId(), MF_BYCOMMAND);
    pimpl_->items_.erase(it);
    return true;
  }
  return false;
}

bool Menu::RemoveItemById(MenuItemId item_id) {
  for (auto it = pimpl_->items_.begin(); it != pimpl_->items_.end(); ++it) {
    if ((*it)->GetId() == item_id) {
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
    if (item->GetId() == item_id) {
      return item;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<MenuItem>> Menu::GetAllItems() const {
  return pimpl_->items_;
}

bool Menu::Open(double x, double y) {
  POINT pt = {static_cast<int>(x), static_cast<int>(y)};

  // Get the host window for menus
  HWND host_window = WindowMessageDispatcher::GetInstance().GetHostWindow();
  if (!host_window) {
    return false;
  }

  // Set the host window as foreground to ensure menu can be displayed
  SetForegroundWindow(host_window);

  // Show the context menu using the host window
  // Note: TrackPopupMenu is a blocking call
  // - WM_INITMENUPOPUP is sent when the menu opens (triggers MenuOpenedEvent)
  // - WM_UNINITMENUPOPUP is sent when the menu closes (triggers MenuClosedEvent)
  TrackPopupMenu(pimpl_->hmenu_, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0,
                 host_window, nullptr);

  return true;
}

bool Menu::Open() {
  POINT pt;
  GetCursorPos(&pt);
  return Open(pt.x, pt.y);
}

bool Menu::Close() {
  if (pimpl_->visible_) {
    // Send WM_CANCELMODE to close any open menus
    HWND host_window = WindowMessageDispatcher::GetInstance().GetHostWindow();
    if (host_window) {
      // This will close the menu and trigger WM_UNINITMENUPOPUP
      SendMessage(host_window, WM_CANCELMODE, 0, 0);
    }
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
