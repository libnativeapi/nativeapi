#include "menu_c.h"
#include "../menu.h"
#include <cstring>
#include <map>
#include <memory>
#include <unordered_map>

using namespace nativeapi;

// Global object storage to manage shared_ptr lifetimes
static std::unordered_map<void*, std::shared_ptr<MenuItem>> g_menu_items;
static std::unordered_map<void*, std::shared_ptr<Menu>> g_menus;

// Internal structures to manage C callbacks
struct MenuItemCallbackData {
  native_menu_item_click_callback_t click_callback;
  native_menu_item_state_changed_callback_t state_changed_callback;
  void* click_user_data;
  void* state_changed_user_data;
};

struct MenuCallbackData {
  native_menu_will_show_callback_t will_show_callback;
  native_menu_did_hide_callback_t did_hide_callback;
  void* will_show_user_data;
  void* did_hide_user_data;
};

// Global maps to store callback data
static std::map<native_menu_item_t, std::unique_ptr<MenuItemCallbackData>> g_menu_item_callbacks;
static std::map<native_menu_t, std::unique_ptr<MenuCallbackData>> g_menu_callbacks;

// Helper functions
static MenuItemType convert_menu_item_type(native_menu_item_type_t type) {
  switch (type) {
    case NATIVE_MENU_ITEM_TYPE_NORMAL:
      return MenuItemType::Normal;
    case NATIVE_MENU_ITEM_TYPE_CHECKBOX:
      return MenuItemType::Checkbox;
    case NATIVE_MENU_ITEM_TYPE_RADIO:
      return MenuItemType::Radio;
    case NATIVE_MENU_ITEM_TYPE_SEPARATOR:
      return MenuItemType::Separator;
    case NATIVE_MENU_ITEM_TYPE_SUBMENU:
      return MenuItemType::Submenu;
    default:
      return MenuItemType::Normal;
  }
}

static native_menu_item_type_t convert_menu_item_type(MenuItemType type) {
  switch (type) {
    case MenuItemType::Normal:
      return NATIVE_MENU_ITEM_TYPE_NORMAL;
    case MenuItemType::Checkbox:
      return NATIVE_MENU_ITEM_TYPE_CHECKBOX;
    case MenuItemType::Radio:
      return NATIVE_MENU_ITEM_TYPE_RADIO;
    case MenuItemType::Separator:
      return NATIVE_MENU_ITEM_TYPE_SEPARATOR;
    case MenuItemType::Submenu:
      return NATIVE_MENU_ITEM_TYPE_SUBMENU;
    default:
      return NATIVE_MENU_ITEM_TYPE_NORMAL;
  }
}

static KeyboardAccelerator convert_keyboard_accelerator(const native_keyboard_accelerator_t* accelerator) {
  int modifiers = 0;
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_CTRL) {
    modifiers |= KeyboardAccelerator::Ctrl;
  }
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_ALT) {
    modifiers |= KeyboardAccelerator::Alt;
  }
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_SHIFT) {
    modifiers |= KeyboardAccelerator::Shift;
  }
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_META) {
    modifiers |= KeyboardAccelerator::Meta;
  }
  return KeyboardAccelerator(accelerator->key, modifiers);
}

static native_keyboard_accelerator_t convert_keyboard_accelerator(const KeyboardAccelerator& accelerator) {
  native_keyboard_accelerator_t result = {};
  
  if (accelerator.modifiers & KeyboardAccelerator::Ctrl) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_CTRL;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Alt) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_ALT;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Shift) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_SHIFT;
  }
  if (accelerator.modifiers & KeyboardAccelerator::Meta) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_META;
  }
  
  strncpy(result.key, accelerator.key.c_str(), sizeof(result.key) - 1);
  result.key[sizeof(result.key) - 1] = '\0';
  
  return result;
}

// MenuItem C API Implementation

native_menu_item_t native_menu_item_create(const char* text, native_menu_item_type_t type) {
  if (!text) return nullptr;
  
  try {
    auto item = MenuItem::Create(text, convert_menu_item_type(type));
    void* handle = item.get();
    g_menu_items[handle] = item;  // Store shared_ptr to keep object alive
    return static_cast<native_menu_item_t>(handle);
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_t native_menu_item_create_separator(void) {
  try {
    auto item = MenuItem::CreateSeparator();
    void* handle = item.get();
    g_menu_items[handle] = item;  // Store shared_ptr to keep object alive
    return static_cast<native_menu_item_t>(handle);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_item_destroy(native_menu_item_t item) {
  if (!item) return;
  
  // Clean up callbacks
  auto it = g_menu_item_callbacks.find(item);
  if (it != g_menu_item_callbacks.end()) {
    g_menu_item_callbacks.erase(it);
  }
  
  // Remove from global storage - this will release the shared_ptr
  auto item_it = g_menu_items.find(item);
  if (item_it != g_menu_items.end()) {
    g_menu_items.erase(item_it);
  }
}

native_menu_item_id_t native_menu_item_get_id(native_menu_item_t item) {
  if (!item) return -1;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return menu_item->id;
  } catch (...) {
    return -1;
  }
}

native_menu_item_type_t native_menu_item_get_type(native_menu_item_t item) {
  if (!item) return NATIVE_MENU_ITEM_TYPE_NORMAL;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return convert_menu_item_type(menu_item->GetType());
  } catch (...) {
    return NATIVE_MENU_ITEM_TYPE_NORMAL;
  }
}

void native_menu_item_set_text(native_menu_item_t item, const char* text) {
  if (!item || !text) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetText(text);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_menu_item_get_text(native_menu_item_t item, char* buffer, size_t buffer_size) {
  if (!item || !buffer || buffer_size == 0) return -1;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    std::string text = menu_item->GetText();
    
    if (text.length() >= buffer_size) {
      return -1;
    }
    
    strncpy(buffer, text.c_str(), buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return static_cast<int>(text.length());
  } catch (...) {
    return -1;
  }
}

void native_menu_item_set_icon(native_menu_item_t item, const char* icon) {
  if (!item || !icon) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetIcon(icon);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_menu_item_get_icon(native_menu_item_t item, char* buffer, size_t buffer_size) {
  if (!item || !buffer || buffer_size == 0) return -1;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    std::string icon = menu_item->GetIcon();
    
    if (icon.length() >= buffer_size) {
      return -1;
    }
    
    strncpy(buffer, icon.c_str(), buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return static_cast<int>(icon.length());
  } catch (...) {
    return -1;
  }
}

void native_menu_item_set_tooltip(native_menu_item_t item, const char* tooltip) {
  if (!item || !tooltip) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetTooltip(tooltip);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_menu_item_get_tooltip(native_menu_item_t item, char* buffer, size_t buffer_size) {
  if (!item || !buffer || buffer_size == 0) return -1;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    std::string tooltip = menu_item->GetTooltip();
    
    if (tooltip.length() >= buffer_size) {
      return -1;
    }
    
    strncpy(buffer, tooltip.c_str(), buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return static_cast<int>(tooltip.length());
  } catch (...) {
    return -1;
  }
}

void native_menu_item_set_accelerator(native_menu_item_t item, const native_keyboard_accelerator_t* accelerator) {
  if (!item || !accelerator) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    KeyboardAccelerator cpp_accelerator = convert_keyboard_accelerator(accelerator);
    menu_item->SetAccelerator(cpp_accelerator);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_get_accelerator(native_menu_item_t item, native_keyboard_accelerator_t* accelerator) {
  if (!item || !accelerator) return false;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    KeyboardAccelerator cpp_accelerator = menu_item->GetAccelerator();
    
    if (cpp_accelerator.key.empty()) {
      return false;
    }
    
    *accelerator = convert_keyboard_accelerator(cpp_accelerator);
    return true;
  } catch (...) {
    return false;
  }
}

void native_menu_item_remove_accelerator(native_menu_item_t item) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->RemoveAccelerator();
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_item_set_enabled(native_menu_item_t item, bool enabled) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetEnabled(enabled);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_is_enabled(native_menu_item_t item) {
  if (!item) return false;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return menu_item->IsEnabled();
  } catch (...) {
    return false;
  }
}

void native_menu_item_set_visible(native_menu_item_t item, bool visible) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetVisible(visible);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_is_visible(native_menu_item_t item) {
  if (!item) return false;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return menu_item->IsVisible();
  } catch (...) {
    return false;
  }
}

void native_menu_item_set_checked(native_menu_item_t item, bool checked) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetChecked(checked);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_is_checked(native_menu_item_t item) {
  if (!item) return false;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return menu_item->IsChecked();
  } catch (...) {
    return false;
  }
}

void native_menu_item_set_radio_group(native_menu_item_t item, int group_id) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetRadioGroup(group_id);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_menu_item_get_radio_group(native_menu_item_t item) {
  if (!item) return -1;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return menu_item->GetRadioGroup();
  } catch (...) {
    return -1;
  }
}

void native_menu_item_set_submenu(native_menu_item_t item, native_menu_t submenu) {
  if (!item || !submenu) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    auto menu_ptr = std::shared_ptr<Menu>(static_cast<Menu*>(submenu));
    menu_item->SetSubmenu(menu_ptr);
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_t native_menu_item_get_submenu(native_menu_item_t item) {
  if (!item) return nullptr;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    auto submenu = menu_item->GetSubmenu();
    return submenu ? static_cast<native_menu_t>(submenu.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

void native_menu_item_remove_submenu(native_menu_item_t item) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->RemoveSubmenu();
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_item_set_on_click(native_menu_item_t item, native_menu_item_click_callback_t callback, void* user_data) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    
    // Get or create callback data
    auto it = g_menu_item_callbacks.find(item);
    if (it == g_menu_item_callbacks.end()) {
      g_menu_item_callbacks[item] = std::make_unique<MenuItemCallbackData>();
    }
    
    auto& callback_data = g_menu_item_callbacks[item];
    callback_data->click_callback = callback;
    callback_data->click_user_data = user_data;
    
    if (callback) {
      menu_item->SetOnClick([item, callback_data = callback_data.get()](const MenuItemSelectedEvent& event) {
        if (callback_data->click_callback) {
          native_menu_item_selected_event_t c_event = {};
          c_event.item_id = event.GetItemId();
          strncpy(c_event.item_text, event.GetItemText().c_str(), sizeof(c_event.item_text) - 1);
          c_event.item_text[sizeof(c_event.item_text) - 1] = '\0';
          
          callback_data->click_callback(&c_event, callback_data->click_user_data);
        }
      });
    } else {
      menu_item->SetOnClick(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_item_set_on_state_changed(native_menu_item_t item, native_menu_item_state_changed_callback_t callback, void* user_data) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    
    // Get or create callback data
    auto it = g_menu_item_callbacks.find(item);
    if (it == g_menu_item_callbacks.end()) {
      g_menu_item_callbacks[item] = std::make_unique<MenuItemCallbackData>();
    }
    
    auto& callback_data = g_menu_item_callbacks[item];
    callback_data->state_changed_callback = callback;
    callback_data->state_changed_user_data = user_data;
    
    if (callback) {
      menu_item->SetOnStateChanged([item, callback_data = callback_data.get()](const MenuItemStateChangedEvent& event) {
        if (callback_data->state_changed_callback) {
          native_menu_item_state_changed_event_t c_event = {};
          c_event.item_id = event.GetItemId();
          c_event.checked = event.IsChecked();
          
          callback_data->state_changed_callback(&c_event, callback_data->state_changed_user_data);
        }
      });
    } else {
      menu_item->SetOnStateChanged(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_trigger(native_menu_item_t item) {
  if (!item) return false;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return menu_item->Trigger();
  } catch (...) {
    return false;
  }
}

// Menu C API Implementation

native_menu_t native_menu_create(void) {
  try {
    auto menu = Menu::Create();
    void* handle = menu.get();
    g_menus[handle] = menu;  // Store shared_ptr to keep object alive
    return static_cast<native_menu_t>(handle);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_destroy(native_menu_t menu) {
  if (!menu) return;
  
  // Clean up callbacks
  auto it = g_menu_callbacks.find(menu);
  if (it != g_menu_callbacks.end()) {
    g_menu_callbacks.erase(it);
  }
  
  // Remove from global storage - this will release the shared_ptr
  auto menu_it = g_menus.find(menu);
  if (menu_it != g_menus.end()) {
    g_menus.erase(menu_it);
  }
}

native_menu_id_t native_menu_get_id(native_menu_t menu) {
  if (!menu) return -1;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->id;
  } catch (...) {
    return -1;
  }
}

void native_menu_add_item(native_menu_t menu, native_menu_item_t item) {
  if (!menu || !item) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    // Get the shared_ptr from global storage instead of creating a new one
    auto item_it = g_menu_items.find(item);
    if (item_it != g_menu_items.end()) {
      menu_ptr->AddItem(item_it->second);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_insert_item(native_menu_t menu, native_menu_item_t item, size_t index) {
  if (!menu || !item) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    // Get the shared_ptr from global storage instead of creating a new one
    auto item_it = g_menu_items.find(item);
    if (item_it != g_menu_items.end()) {
      menu_ptr->InsertItem(index, item_it->second);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_remove_item(native_menu_t menu, native_menu_item_t item) {
  if (!menu || !item) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto item_ptr = std::shared_ptr<MenuItem>(static_cast<MenuItem*>(item));
    return menu_ptr->RemoveItem(item_ptr);
  } catch (...) {
    return false;
  }
}

bool native_menu_remove_item_by_id(native_menu_t menu, native_menu_item_id_t item_id) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->RemoveItemById(item_id);
  } catch (...) {
    return false;
  }
}

bool native_menu_remove_item_at(native_menu_t menu, size_t index) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->RemoveItemAt(index);
  } catch (...) {
    return false;
  }
}

void native_menu_clear(native_menu_t menu) {
  if (!menu) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->Clear();
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_add_separator(native_menu_t menu) {
  if (!menu) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->AddSeparator();
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_insert_separator(native_menu_t menu, size_t index) {
  if (!menu) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->InsertSeparator(index);
  } catch (...) {
    // Ignore exceptions
  }
}

size_t native_menu_get_item_count(native_menu_t menu) {
  if (!menu) return 0;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->GetItemCount();
  } catch (...) {
    return 0;
  }
}

native_menu_item_t native_menu_get_item_at(native_menu_t menu, size_t index) {
  if (!menu) return nullptr;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto item = menu_ptr->GetItemAt(index);
    return item ? static_cast<native_menu_item_t>(item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_t native_menu_get_item_by_id(native_menu_t menu, native_menu_item_id_t item_id) {
  if (!menu) return nullptr;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto item = menu_ptr->GetItemById(item_id);
    return item ? static_cast<native_menu_item_t>(item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_list_t native_menu_get_all_items(native_menu_t menu) {
  native_menu_item_list_t result = { nullptr, 0 };
  
  if (!menu) return result;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto items = menu_ptr->GetAllItems();
    
    if (items.empty()) {
      return result;
    }
    
    result.items = static_cast<native_menu_item_t*>(malloc(items.size() * sizeof(native_menu_item_t)));
    if (!result.items) {
      return result;
    }
    
    result.count = items.size();
    for (size_t i = 0; i < items.size(); ++i) {
      result.items[i] = static_cast<native_menu_item_t>(items[i].get());
    }
    
    return result;
  } catch (...) {
    if (result.items) {
      free(result.items);
      result.items = nullptr;
      result.count = 0;
    }
    return result;
  }
}

native_menu_item_t native_menu_find_item_by_text(native_menu_t menu, const char* text) {
  if (!menu || !text) return nullptr;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto item = menu_ptr->FindItemByText(text);
    return item ? static_cast<native_menu_item_t>(item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

bool native_menu_show_as_context_menu(native_menu_t menu, double x, double y) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->ShowAsContextMenu(x, y);
  } catch (...) {
    return false;
  }
}

bool native_menu_show_as_context_menu_default(native_menu_t menu) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->ShowAsContextMenu();
  } catch (...) {
    return false;
  }
}

bool native_menu_close(native_menu_t menu) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->Close();
  } catch (...) {
    return false;
  }
}

bool native_menu_is_visible(native_menu_t menu) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->IsVisible();
  } catch (...) {
    return false;
  }
}

void native_menu_set_enabled(native_menu_t menu, bool enabled) {
  if (!menu) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->SetEnabled(enabled);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_is_enabled(native_menu_t menu) {
  if (!menu) return false;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->IsEnabled();
  } catch (...) {
    return false;
  }
}

void native_menu_set_on_will_show(native_menu_t menu, native_menu_will_show_callback_t callback, void* user_data) {
  if (!menu) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    
    // Get or create callback data
    auto it = g_menu_callbacks.find(menu);
    if (it == g_menu_callbacks.end()) {
      g_menu_callbacks[menu] = std::make_unique<MenuCallbackData>();
    }
    
    auto& callback_data = g_menu_callbacks[menu];
    callback_data->will_show_callback = callback;
    callback_data->will_show_user_data = user_data;
    
    if (callback) {
      menu_ptr->SetOnMenuWillShow([menu, callback_data = callback_data.get()]() {
        if (callback_data->will_show_callback) {
          auto menu_ptr = static_cast<Menu*>(menu);
          callback_data->will_show_callback(menu_ptr->id, callback_data->will_show_user_data);
        }
      });
    } else {
      menu_ptr->SetOnMenuWillShow(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_set_on_did_hide(native_menu_t menu, native_menu_did_hide_callback_t callback, void* user_data) {
  if (!menu) return;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    
    // Get or create callback data
    auto it = g_menu_callbacks.find(menu);
    if (it == g_menu_callbacks.end()) {
      g_menu_callbacks[menu] = std::make_unique<MenuCallbackData>();
    }
    
    auto& callback_data = g_menu_callbacks[menu];
    callback_data->did_hide_callback = callback;
    callback_data->did_hide_user_data = user_data;
    
    if (callback) {
      menu_ptr->SetOnMenuDidHide([menu, callback_data = callback_data.get()]() {
        if (callback_data->did_hide_callback) {
          auto menu_ptr = static_cast<Menu*>(menu);
          callback_data->did_hide_callback(menu_ptr->id, callback_data->did_hide_user_data);
        }
      });
    } else {
      menu_ptr->SetOnMenuDidHide(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_item_t native_menu_create_and_add_item(native_menu_t menu, const char* text, native_menu_item_type_t type) {
  if (!menu || !text) return nullptr;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto item = menu_ptr->CreateAndAddItem(text);
    if (item && type != NATIVE_MENU_ITEM_TYPE_NORMAL) {
      // The CreateAndAddItem only creates normal items, so we need to handle other types differently
      // For now, just return the normal item - this could be enhanced later
    }
    return item ? static_cast<native_menu_item_t>(item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_t native_menu_create_and_add_submenu(native_menu_t menu, const char* text, native_menu_t submenu) {
  if (!menu || !text || !submenu) return nullptr;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto submenu_ptr = std::shared_ptr<Menu>(static_cast<Menu*>(submenu));
    auto item = menu_ptr->CreateAndAddSubmenu(text, submenu_ptr);
    return item ? static_cast<native_menu_item_t>(item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

// Utility functions

void native_menu_item_list_free(native_menu_item_list_t list) {
  if (list.items) {
    free(list.items);
  }
}

int native_keyboard_accelerator_to_string(const native_keyboard_accelerator_t* accelerator, char* buffer, size_t buffer_size) {
  if (!accelerator || !buffer || buffer_size == 0) return -1;
  
  try {
    KeyboardAccelerator cpp_accelerator = convert_keyboard_accelerator(accelerator);
    std::string str = cpp_accelerator.ToString();
    
    if (str.length() >= buffer_size) {
      return -1;
    }
    
    strncpy(buffer, str.c_str(), buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return static_cast<int>(str.length());
  } catch (...) {
    return -1;
  }
}