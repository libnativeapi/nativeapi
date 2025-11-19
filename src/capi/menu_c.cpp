#include "menu_c.h"
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include "../image.h"
#include "../menu.h"
#include "../placement.h"
#include "../positioning_strategy.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Internal structures to manage event listeners

// Event listener data structures
struct EventListenerData {
  native_menu_item_event_callback_t callback;
  void* user_data;
};

struct MenuEventListenerData {
  native_menu_event_callback_t callback;
  void* user_data;
};

// Global maps to store event listeners
// Key is the C++ listener ID returned by AddListener
static std::map<native_menu_item_t, std::map<size_t, std::shared_ptr<EventListenerData>>>
    g_menu_item_listeners;
static std::map<native_menu_t, std::map<size_t, std::shared_ptr<MenuEventListenerData>>>
    g_menu_listeners;

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

static KeyboardAccelerator convert_keyboard_accelerator(
    const native_keyboard_accelerator_t* accelerator) {
  ModifierKey modifiers = ModifierKey::None;
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_CTRL) {
    modifiers |= ModifierKey::Ctrl;
  }
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_ALT) {
    modifiers |= ModifierKey::Alt;
  }
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_SHIFT) {
    modifiers |= ModifierKey::Shift;
  }
  if (accelerator->modifiers & NATIVE_ACCELERATOR_MODIFIER_META) {
    modifiers |= ModifierKey::Meta;
  }
  return KeyboardAccelerator(accelerator->key, modifiers);
}

static native_keyboard_accelerator_t convert_keyboard_accelerator(
    const KeyboardAccelerator& accelerator) {
  native_keyboard_accelerator_t result = {};

  if ((accelerator.modifiers & ModifierKey::Ctrl) != ModifierKey::None) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_CTRL;
  }
  if ((accelerator.modifiers & ModifierKey::Alt) != ModifierKey::None) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_ALT;
  }
  if ((accelerator.modifiers & ModifierKey::Shift) != ModifierKey::None) {
    result.modifiers |= NATIVE_ACCELERATOR_MODIFIER_SHIFT;
  }
  if ((accelerator.modifiers & ModifierKey::Meta) != ModifierKey::None) {
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

native_menu_item_t native_menu_item_create(const char* label, native_menu_item_type_t type) {
  if (!label)
    return nullptr;

  try {
    auto menu_item_raw = new MenuItem(label, convert_menu_item_type(type));
    return static_cast<native_menu_item_t>(menu_item_raw);
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_t native_menu_item_create_separator(void) {
  try {
    auto menu_item_raw = new MenuItem("", MenuItemType::Separator);
    return static_cast<native_menu_item_t>(menu_item_raw);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_item_destroy(native_menu_item_t menu_item) {
  if (!menu_item)
    return;

  // Remove event listeners first
  auto listeners_it = g_menu_item_listeners.find(menu_item);
  if (listeners_it != g_menu_item_listeners.end()) {
    g_menu_item_listeners.erase(listeners_it);
  }

  // Delete MenuItem instance
  auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
  delete menu_item_ptr;
}

native_menu_item_id_t native_menu_item_get_id(native_menu_item_t menu_item) {
  if (!menu_item)
    return -1;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (!menu_item_ptr)
      return -1;
    return menu_item_ptr->GetId();
  } catch (...) {
    return -1;
  }
}

native_menu_item_type_t native_menu_item_get_type(native_menu_item_t menu_item) {
  if (!menu_item)
    return NATIVE_MENU_ITEM_TYPE_NORMAL;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    return convert_menu_item_type(menu_item_ptr->GetType());
  } catch (...) {
    return NATIVE_MENU_ITEM_TYPE_NORMAL;
  }
}

void native_menu_item_set_label(native_menu_item_t menu_item, const char* label) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (label) {
      menu_item_ptr->SetLabel(std::string(label));
    } else {
      menu_item_ptr->SetLabel(std::nullopt);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

char* native_menu_item_get_label(native_menu_item_t menu_item) {
  if (!menu_item)
    return nullptr;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    auto labelOpt = menu_item_ptr->GetLabel();

    if (!labelOpt.has_value()) {
      return nullptr;
    }

    const std::string& label = labelOpt.value();
    return to_c_str(label);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_item_set_icon(native_menu_item_t menu_item, native_image_t image) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (image) {
      // Extract the shared_ptr from the native_image_t handle
      auto image_ptr = static_cast<std::shared_ptr<Image>*>(image);
      menu_item_ptr->SetIcon(*image_ptr);
    } else {
      menu_item_ptr->SetIcon(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

native_image_t native_menu_item_get_icon(native_menu_item_t menu_item) {
  if (!menu_item)
    return nullptr;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    auto image = menu_item_ptr->GetIcon();

    if (!image) {
      return nullptr;
    }

    // Create a new shared_ptr wrapper for the C API
    return new std::shared_ptr<Image>(image);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_item_set_tooltip(native_menu_item_t menu_item, const char* tooltip) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (tooltip) {
      menu_item_ptr->SetTooltip(std::string(tooltip));
    } else {
      menu_item_ptr->SetTooltip(std::nullopt);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

char* native_menu_item_get_tooltip(native_menu_item_t menu_item) {
  if (!menu_item)
    return nullptr;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    auto tooltipOpt = menu_item_ptr->GetTooltip();

    if (!tooltipOpt.has_value()) {
      return nullptr;
    }

    const std::string& tooltip = tooltipOpt.value();
    return to_c_str(tooltip);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_item_set_accelerator(native_menu_item_t menu_item,
                                      const native_keyboard_accelerator_t* accelerator) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (accelerator) {
      KeyboardAccelerator cpp_accelerator = convert_keyboard_accelerator(accelerator);
      menu_item_ptr->SetAccelerator(cpp_accelerator);
    } else {
      menu_item_ptr->SetAccelerator(std::nullopt);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_get_accelerator(native_menu_item_t menu_item,
                                      native_keyboard_accelerator_t* accelerator) {
  if (!menu_item || !accelerator)
    return false;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    KeyboardAccelerator cpp_accelerator = menu_item_ptr->GetAccelerator();

    if (cpp_accelerator.key.empty()) {
      return false;
    }

    *accelerator = convert_keyboard_accelerator(cpp_accelerator);
    return true;
  } catch (...) {
    return false;
  }
}

void native_menu_item_set_enabled(native_menu_item_t menu_item, bool enabled) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    menu_item_ptr->SetEnabled(enabled);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_item_is_enabled(native_menu_item_t menu_item) {
  if (!menu_item)
    return false;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    return menu_item_ptr->IsEnabled();
  } catch (...) {
    return false;
  }
}

void native_menu_item_set_state(native_menu_item_t menu_item, native_menu_item_state_t state) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    menu_item_ptr->SetState(convert_menu_item_state(state));
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_item_state_t native_menu_item_get_state(native_menu_item_t menu_item) {
  if (!menu_item)
    return NATIVE_MENU_ITEM_STATE_UNCHECKED;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    return convert_menu_item_state(menu_item_ptr->GetState());
  } catch (...) {
    return NATIVE_MENU_ITEM_STATE_UNCHECKED;
  }
}

void native_menu_item_set_radio_group(native_menu_item_t menu_item, int group_id) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    menu_item_ptr->SetRadioGroup(group_id);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_menu_item_get_radio_group(native_menu_item_t menu_item) {
  if (!menu_item)
    return -1;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    return menu_item_ptr->GetRadioGroup();
  } catch (...) {
    return -1;
  }
}

void native_menu_item_set_submenu(native_menu_item_t menu_item, native_menu_t submenu) {
  if (!menu_item)
    return;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (!menu_item_ptr)
      return;
    if (submenu) {
      // IMPORTANT: Do NOT create an owning shared_ptr from a raw pointer here.
      // Menu objects are owned by the C API caller. Creating an owning shared_ptr
      // would introduce a separate control block and cause double-deletion during shutdown.
      // Use a non-owning aliasing shared_ptr with an empty deleter instead.
      auto menu_raw = static_cast<Menu*>(submenu);
      auto menu_ptr = std::shared_ptr<Menu>(menu_raw, [](Menu*) {});
      menu_item_ptr->SetSubmenu(menu_ptr);
    } else {
      menu_item_ptr->SetSubmenu(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_t native_menu_item_get_submenu(native_menu_item_t menu_item) {
  if (!menu_item)
    return nullptr;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    auto submenu = menu_item_ptr->GetSubmenu();
    return submenu ? static_cast<native_menu_t>(submenu.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

// New event listener API implementation
int native_menu_item_add_listener(native_menu_item_t menu_item,
                                  native_menu_item_event_type_t event_type,
                                  native_menu_item_event_callback_t callback,
                                  void* user_data) {
  if (!menu_item || !callback)
    return -1;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);

    // Create listener data
    auto listener_data = std::make_shared<EventListenerData>();
    listener_data->callback = callback;
    listener_data->user_data = user_data;

    // Add the appropriate event listener based on event type
    size_t cpp_listener_id = 0;
    if (event_type == NATIVE_MENU_ITEM_EVENT_CLICKED) {
      cpp_listener_id = menu_item_ptr->AddListener<MenuItemClickedEvent>(
          [listener_data](const MenuItemClickedEvent& event) {
            if (listener_data && listener_data->callback) {
              native_menu_item_clicked_event_t c_event = {};
              c_event.item_id = event.GetItemId();

              listener_data->callback(&c_event, listener_data->user_data);
            }
          });
    } else if (event_type == NATIVE_MENU_ITEM_EVENT_SUBMENU_OPENED) {
      cpp_listener_id = menu_item_ptr->AddListener<MenuItemSubmenuOpenedEvent>(
          [listener_data](const MenuItemSubmenuOpenedEvent& event) {
            if (listener_data && listener_data->callback) {
              native_menu_item_submenu_opened_event_t c_event = {};
              c_event.item_id = event.GetItemId();

              listener_data->callback(&c_event, listener_data->user_data);
            }
          });
    } else if (event_type == NATIVE_MENU_ITEM_EVENT_SUBMENU_CLOSED) {
      cpp_listener_id = menu_item_ptr->AddListener<MenuItemSubmenuClosedEvent>(
          [listener_data](const MenuItemSubmenuClosedEvent& event) {
            if (listener_data && listener_data->callback) {
              native_menu_item_submenu_closed_event_t c_event = {};
              c_event.item_id = event.GetItemId();

              listener_data->callback(&c_event, listener_data->user_data);
            }
          });
    }

    // Store the listener data using C++ listener ID as key
    g_menu_item_listeners[menu_item][cpp_listener_id] = listener_data;

    return static_cast<int>(cpp_listener_id);
  } catch (...) {
    return -1;
  }
}

bool native_menu_item_remove_listener(native_menu_item_t menu_item, int listener_id) {
  if (!menu_item)
    return false;

  try {
    auto menu_item_ptr = static_cast<MenuItem*>(menu_item);
    if (!menu_item_ptr)
      return false;

    // Convert C API listener_id (int) to C++ listener_id (size_t)
    size_t cpp_listener_id = static_cast<size_t>(listener_id);

    auto menu_item_it = g_menu_item_listeners.find(menu_item);
    if (menu_item_it != g_menu_item_listeners.end()) {
      auto listener_it = menu_item_it->second.find(cpp_listener_id);
      if (listener_it != menu_item_it->second.end()) {
        // Remove the C++ listener first
        menu_item_ptr->RemoveListener(cpp_listener_id);

        // Remove the listener data
        menu_item_it->second.erase(listener_it);

        // If no more listeners for this menu_item, remove the menu_item entry
        if (menu_item_it->second.empty()) {
          g_menu_item_listeners.erase(menu_item_it);
        }

        return true;
      }
    }
    return false;
  } catch (...) {
    return false;
  }
}

// Menu C API Implementation

native_menu_t native_menu_create(void) {
  try {
    auto menu_raw = new Menu();
    return static_cast<native_menu_t>(menu_raw);
  } catch (...) {
    return nullptr;
  }
}

void native_menu_destroy(native_menu_t menu) {
  if (!menu)
    return;

  // Remove event listeners first
  auto listeners_it = g_menu_listeners.find(menu);
  if (listeners_it != g_menu_listeners.end()) {
    g_menu_listeners.erase(listeners_it);
  }

  // Delete Menu instance
  auto menu_ptr = static_cast<Menu*>(menu);
  delete menu_ptr;
}

native_menu_id_t native_menu_get_id(native_menu_t menu) {
  if (!menu)
    return -1;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    if (!menu_ptr)
      return -1;
    return menu_ptr->GetId();
  } catch (...) {
    return -1;
  }
}

void native_menu_add_item(native_menu_t menu, native_menu_item_t menu_item) {
  if (!menu || !menu_item)
    return;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    if (!menu_ptr)
      return;
    // IMPORTANT: Do NOT create an owning shared_ptr from a raw pointer here.
    // MenuItem objects are owned by the C API caller. Creating an owning shared_ptr
    // would introduce a separate control block and cause double-deletion during shutdown.
    // Use a non-owning aliasing shared_ptr with an empty deleter instead.
    auto menu_item_raw = static_cast<MenuItem*>(menu_item);
    auto menu_item_ptr = std::shared_ptr<MenuItem>(menu_item_raw, [](MenuItem*) {});
    menu_ptr->AddItem(menu_item_ptr);
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_insert_item(native_menu_t menu, native_menu_item_t menu_item, size_t index) {
  if (!menu || !menu_item)
    return;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    if (!menu_ptr)
      return;
    // IMPORTANT: Do NOT create an owning shared_ptr from a raw pointer here.
    // MenuItem objects are owned by the C API caller. Creating an owning shared_ptr
    // would introduce a separate control block and cause double-deletion during shutdown.
    // Use a non-owning aliasing shared_ptr with an empty deleter instead.
    auto menu_item_raw = static_cast<MenuItem*>(menu_item);
    auto menu_item_ptr = std::shared_ptr<MenuItem>(menu_item_raw, [](MenuItem*) {});
    menu_ptr->InsertItem(index, menu_item_ptr);
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_menu_remove_item(native_menu_t menu, native_menu_item_t menu_item) {
  if (!menu || !menu_item)
    return false;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    if (!menu_ptr)
      return false;
    // IMPORTANT: Do NOT create an owning shared_ptr from a raw pointer here.
    // MenuItem objects are owned by the C API caller. Creating an owning shared_ptr
    // would introduce a separate control block and cause double-deletion during shutdown.
    // Use a non-owning aliasing shared_ptr with an empty deleter instead.
    auto menu_item_raw = static_cast<MenuItem*>(menu_item);
    auto menu_item_ptr = std::shared_ptr<MenuItem>(menu_item_raw, [](MenuItem*) {});
    return menu_ptr->RemoveItem(menu_item_ptr);
  } catch (...) {
    return false;
  }
}

bool native_menu_remove_item_by_id(native_menu_t menu, native_menu_item_id_t item_id) {
  if (!menu)
    return false;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->RemoveItemById(static_cast<MenuItemId>(item_id));
  } catch (...) {
    return false;
  }
}

bool native_menu_remove_item_at(native_menu_t menu, size_t index) {
  if (!menu)
    return false;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->RemoveItemAt(index);
  } catch (...) {
    return false;
  }
}

void native_menu_clear(native_menu_t menu) {
  if (!menu)
    return;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->Clear();
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_add_separator(native_menu_t menu) {
  if (!menu)
    return;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->AddSeparator();
  } catch (...) {
    // Ignore exceptions
  }
}

void native_menu_insert_separator(native_menu_t menu, size_t index) {
  if (!menu)
    return;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    menu_ptr->InsertSeparator(index);
  } catch (...) {
    // Ignore exceptions
  }
}

size_t native_menu_get_item_count(native_menu_t menu) {
  if (!menu)
    return 0;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->GetItemCount();
  } catch (...) {
    return 0;
  }
}

native_menu_item_t native_menu_get_item_at(native_menu_t menu, size_t index) {
  if (!menu)
    return nullptr;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto menu_item = menu_ptr->GetItemAt(index);
    return menu_item ? static_cast<native_menu_item_t>(menu_item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_t native_menu_get_item_by_id(native_menu_t menu, native_menu_item_id_t item_id) {
  if (!menu)
    return nullptr;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto menu_item = menu_ptr->GetItemById(static_cast<MenuItemId>(item_id));
    return menu_item ? static_cast<native_menu_item_t>(menu_item.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

native_menu_item_list_t native_menu_get_all_items(native_menu_t menu) {
  native_menu_item_list_t result = {nullptr, 0};

  if (!menu)
    return result;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto items = menu_ptr->GetAllItems();

    if (items.empty()) {
      return result;
    }

    result.items =
        static_cast<native_menu_item_t*>(malloc(items.size() * sizeof(native_menu_item_t)));
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

bool native_menu_open(native_menu_t menu,
                      native_positioning_strategy_t strategy,
                      native_placement_t placement) {
  if (!menu || !strategy)
    return false;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    auto strategy_ptr = static_cast<PositioningStrategy*>(strategy);

    // Convert C placement enum to C++ placement enum
    Placement cpp_placement;
    switch (placement) {
      case NATIVE_PLACEMENT_TOP:
        cpp_placement = Placement::Top;
        break;
      case NATIVE_PLACEMENT_TOP_START:
        cpp_placement = Placement::TopStart;
        break;
      case NATIVE_PLACEMENT_TOP_END:
        cpp_placement = Placement::TopEnd;
        break;
      case NATIVE_PLACEMENT_RIGHT:
        cpp_placement = Placement::Right;
        break;
      case NATIVE_PLACEMENT_RIGHT_START:
        cpp_placement = Placement::RightStart;
        break;
      case NATIVE_PLACEMENT_RIGHT_END:
        cpp_placement = Placement::RightEnd;
        break;
      case NATIVE_PLACEMENT_BOTTOM:
        cpp_placement = Placement::Bottom;
        break;
      case NATIVE_PLACEMENT_BOTTOM_START:
        cpp_placement = Placement::BottomStart;
        break;
      case NATIVE_PLACEMENT_BOTTOM_END:
        cpp_placement = Placement::BottomEnd;
        break;
      case NATIVE_PLACEMENT_LEFT:
        cpp_placement = Placement::Left;
        break;
      case NATIVE_PLACEMENT_LEFT_START:
        cpp_placement = Placement::LeftStart;
        break;
      case NATIVE_PLACEMENT_LEFT_END:
        cpp_placement = Placement::LeftEnd;
        break;
      default:
        cpp_placement = Placement::BottomStart;
        break;
    }

    return menu_ptr->Open(*strategy_ptr, cpp_placement);
  } catch (...) {
    return false;
  }
}

bool native_menu_close(native_menu_t menu) {
  if (!menu)
    return false;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    return menu_ptr->Close();
  } catch (...) {
    return false;
  }
}

// New menu event listener API implementation
int native_menu_add_listener(native_menu_t menu,
                             native_menu_event_type_t event_type,
                             native_menu_event_callback_t callback,
                             void* user_data) {
  if (!menu || !callback)
    return -1;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);

    // Create listener data
    auto listener_data = std::make_shared<MenuEventListenerData>();
    listener_data->callback = callback;
    listener_data->user_data = user_data;

    // Add the appropriate event listener based on event type
    size_t cpp_listener_id = 0;
    if (event_type == NATIVE_MENU_EVENT_OPENED) {
      cpp_listener_id =
          menu_ptr->AddListener<MenuOpenedEvent>([listener_data](const MenuOpenedEvent& event) {
            if (listener_data && listener_data->callback) {
              native_menu_opened_event_t c_event = {};
              c_event.menu_id = event.GetMenuId();

              listener_data->callback(&c_event, listener_data->user_data);
            }
          });
    } else if (event_type == NATIVE_MENU_EVENT_CLOSED) {
      cpp_listener_id =
          menu_ptr->AddListener<MenuClosedEvent>([listener_data](const MenuClosedEvent& event) {
            if (listener_data && listener_data->callback) {
              native_menu_closed_event_t c_event = {};
              c_event.menu_id = event.GetMenuId();

              listener_data->callback(&c_event, listener_data->user_data);
            }
          });
    }

    // Store the listener data using C++ listener ID as key
    g_menu_listeners[menu][cpp_listener_id] = listener_data;

    return static_cast<int>(cpp_listener_id);
  } catch (...) {
    return -1;
  }
}

bool native_menu_remove_listener(native_menu_t menu, int listener_id) {
  if (!menu)
    return false;

  try {
    auto menu_ptr = static_cast<Menu*>(menu);
    if (!menu_ptr)
      return false;

    // Convert C API listener_id (int) to C++ listener_id (size_t)
    size_t cpp_listener_id = static_cast<size_t>(listener_id);

    auto menu_it = g_menu_listeners.find(menu);
    if (menu_it != g_menu_listeners.end()) {
      auto listener_it = menu_it->second.find(cpp_listener_id);
      if (listener_it != menu_it->second.end()) {
        // Remove the C++ listener first
        menu_ptr->RemoveListener(cpp_listener_id);

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

// Utility functions

void native_menu_item_list_free(native_menu_item_list_t list) {
  if (list.items) {
    free(list.items);
  }
}

char* native_keyboard_accelerator_to_string(const native_keyboard_accelerator_t* accelerator) {
  if (!accelerator)
    return nullptr;

  try {
    KeyboardAccelerator cpp_accelerator = convert_keyboard_accelerator(accelerator);
    std::string str = cpp_accelerator.ToString();
    return to_c_str(str);
  } catch (...) {
    return nullptr;
  }
}
