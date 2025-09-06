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

// Internal structures to manage event listeners

// Event listener data structures
struct EventListenerData {
  native_menu_item_event_callback_t callback;
  void* user_data;
  int listener_id;
};

struct MenuEventListenerData {
  native_menu_event_callback_t callback;
  void* user_data;
  int listener_id;
};

// Global maps to store event listeners
static std::map<native_menu_item_t, std::map<int, std::unique_ptr<EventListenerData>>> g_menu_item_listeners;
static std::map<native_menu_t, std::map<int, std::unique_ptr<MenuEventListenerData>>> g_menu_listeners;

// Global listener ID counter
static std::atomic<int> g_next_listener_id{1};

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

static MenuItemState convert_menu_item_state(native_menu_item_state_t state) {
  switch (state) {
    case NATIVE_MENU_ITEM_STATE_UNCHECKED:
      return MenuItemState::Unchecked;
    case NATIVE_MENU_ITEM_STATE_CHECKED:
      return MenuItemState::Checked;
    case NATIVE_MENU_ITEM_STATE_MIXED:
      return MenuItemState::Mixed;
    default:
      return MenuItemState::Unchecked;
  }
}

static native_menu_item_state_t convert_menu_item_state(MenuItemState state) {
  switch (state) {
    case MenuItemState::Unchecked:
      return NATIVE_MENU_ITEM_STATE_UNCHECKED;
    case MenuItemState::Checked:
      return NATIVE_MENU_ITEM_STATE_CHECKED;
    case MenuItemState::Mixed:
      return NATIVE_MENU_ITEM_STATE_MIXED;
    default:
      return NATIVE_MENU_ITEM_STATE_UNCHECKED;
  }
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

void native_menu_item_set_state(native_menu_item_t item, native_menu_item_state_t state) {
  if (!item) return;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    menu_item->SetState(convert_menu_item_state(state));
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_item_state_t native_menu_item_get_state(native_menu_item_t item) {
  if (!item) return NATIVE_MENU_ITEM_STATE_UNCHECKED;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    return convert_menu_item_state(menu_item->GetState());
  } catch (...) {
    return NATIVE_MENU_ITEM_STATE_UNCHECKED;
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



// New event listener API implementation
int native_menu_item_add_listener(native_menu_item_t item, native_menu_item_event_type_t event_type, native_menu_item_event_callback_t callback, void* user_data) {
  if (!item || !callback) return -1;
  
  try {
    auto menu_item = static_cast<MenuItem*>(item);
    int listener_id = g_next_listener_id++;
    
    // Create listener data
    auto listener_data = std::make_unique<EventListenerData>();
    listener_data->callback = callback;
    listener_data->user_data = user_data;
    listener_data->listener_id = listener_id;
    
    // Store the listener data
    g_menu_item_listeners[item][listener_id] = std::move(listener_data);
    
    // Add the appropriate event listener based on event type
    if (event_type == NATIVE_MENU_ITEM_EVENT_CLICKED) {
      menu_item->AddListener<MenuItemClickedEvent>([item, listener_id](const MenuItemClickedEvent& event) {
        // Find the listener data
        auto item_it = g_menu_item_listeners.find(item);
        if (item_it != g_menu_item_listeners.end()) {
          auto listener_it = item_it->second.find(listener_id);
          if (listener_it != item_it->second.end()) {
            native_menu_item_clicked_event_t c_event = {};
            c_event.item_id = event.GetItemId();
            strncpy(c_event.item_text, event.GetItemText().c_str(), sizeof(c_event.item_text) - 1);
            c_event.item_text[sizeof(c_event.item_text) - 1] = '\0';
            
            listener_it->second->callback(&c_event, listener_it->second->user_data);
          }
        }
      });
    } else if (event_type == NATIVE_MENU_ITEM_EVENT_SUBMENU_OPENED) {
      menu_item->AddListener<MenuItemSubmenuOpenedEvent>([item, listener_id](const MenuItemSubmenuOpenedEvent& event) {
        // Find the listener data
        auto item_it = g_menu_item_listeners.find(item);
        if (item_it != g_menu_item_listeners.end()) {
          auto listener_it = item_it->second.find(listener_id);
          if (listener_it != item_it->second.end()) {
            native_menu_item_submenu_opened_event_t c_event = {};
            c_event.item_id = event.GetItemId();
            
            listener_it->second->callback(&c_event, listener_it->second->user_data);
          }
        }
      });
    } else if (event_type == NATIVE_MENU_ITEM_EVENT_SUBMENU_CLOSED) {
      menu_item->AddListener<MenuItemSubmenuClosedEvent>([item, listener_id](const MenuItemSubmenuClosedEvent& event) {
        // Find the listener data
        auto item_it = g_menu_item_listeners.find(item);
        if (item_it != g_menu_item_listeners.end()) {
          auto listener_it = item_it->second.find(listener_id);
          if (listener_it != item_it->second.end()) {
            native_menu_item_submenu_closed_event_t c_event = {};
            c_event.item_id = event.GetItemId();
            
            listener_it->second->callback(&c_event, listener_it->second->user_data);
          }
        }
      });
    }
    
    return listener_id;
  } catch (...) {
    return -1;
  }
}

bool native_menu_item_remove_listener(native_menu_item_t item, int listener_id) {
  if (!item) return false;
  
  try {
    auto item_it = g_menu_item_listeners.find(item);
    if (item_it != g_menu_item_listeners.end()) {
      auto listener_it = item_it->second.find(listener_id);
      if (listener_it != item_it->second.end()) {
        // Remove the listener data
        item_it->second.erase(listener_it);
        
        // If no more listeners for this item, remove the item entry
        if (item_it->second.empty()) {
          g_menu_item_listeners.erase(item_it);
        }
        
        return true;
      }
    }
    return false;
  } catch (...) {
    return false;
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



// New menu event listener API implementation
int native_menu_add_listener(native_menu_t menu, native_menu_event_type_t event_type, native_menu_event_callback_t callback, void* user_data) {
  if (!menu || !callback) return -1;
  
  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    int listener_id = g_next_listener_id++;
    
    // Create listener data
    auto listener_data = std::make_unique<MenuEventListenerData>();
    listener_data->callback = callback;
    listener_data->user_data = user_data;
    listener_data->listener_id = listener_id;
    
    // Store the listener data
    g_menu_listeners[menu][listener_id] = std::move(listener_data);
    
    // Add the appropriate event listener based on event type
    if (event_type == NATIVE_MENU_EVENT_OPENED) {
      menu_ptr->AddListener<MenuOpenedEvent>([menu, listener_id](const MenuOpenedEvent& event) {
        // Find the listener data
        auto menu_it = g_menu_listeners.find(menu);
        if (menu_it != g_menu_listeners.end()) {
          auto listener_it = menu_it->second.find(listener_id);
          if (listener_it != menu_it->second.end()) {
            native_menu_opened_event_t c_event = {};
            c_event.menu_id = event.GetMenuId();
            
            listener_it->second->callback(&c_event, listener_it->second->user_data);
          }
        }
      });
    } else if (event_type == NATIVE_MENU_EVENT_CLOSED) {
      menu_ptr->AddListener<MenuClosedEvent>([menu, listener_id](const MenuClosedEvent& event) {
        // Find the listener data
        auto menu_it = g_menu_listeners.find(menu);
        if (menu_it != g_menu_listeners.end()) {
          auto listener_it = menu_it->second.find(listener_id);
          if (listener_it != menu_it->second.end()) {
            native_menu_closed_event_t c_event = {};
            c_event.menu_id = event.GetMenuId();
            
            listener_it->second->callback(&c_event, listener_it->second->user_data);
          }
        }
      });
    }
    
    return listener_id;
  } catch (...) {
    return -1;
  }
}

bool native_menu_remove_listener(native_menu_t menu, int listener_id) {
  if (!menu) return false;
  
  try {
    auto menu_it = g_menu_listeners.find(menu);
    if (menu_it != g_menu_listeners.end()) {
      auto listener_it = menu_it->second.find(listener_id);
      if (listener_it != menu_it->second.end()) {
        // Remove the listener data
        menu_it->second.erase(listener_it);
        
        // If no more listeners for this menu, remove the menu entry
        if (menu_it->second.empty()) {
          g_menu_listeners.erase(menu_it);
        }
        
        return true;
      }
    }
    return false;
  } catch (...) {
    return false;
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