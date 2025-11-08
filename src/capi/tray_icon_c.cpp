#include "tray_icon_c.h"
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include "../image.h"
#include "../tray_icon.h"
#include "../tray_icon_event.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Internal structure to manage C event listeners
struct TrayIconListenerData {
  native_tray_icon_event_type_t event_type;
  native_tray_icon_event_callback_t callback;
  void* user_data;
  size_t listener_id;
};

// Global maps to store listener data
static std::map<native_tray_icon_t, std::vector<std::shared_ptr<TrayIconListenerData>>>
    g_tray_icon_listeners;
static std::atomic<int> g_tray_icon_next_listener_id{1};

// TrayIcon C API Implementation

native_tray_icon_t native_tray_icon_create(void) {
  try {
    auto tray_icon_raw = new TrayIcon();
    return static_cast<native_tray_icon_t>(tray_icon_raw);
  } catch (...) {
    return nullptr;
  }
}

native_tray_icon_t native_tray_icon_create_from_native(void* native_tray) {
  if (!native_tray)
    return nullptr;

  try {
    auto tray_icon_raw = new TrayIcon(native_tray);
    return static_cast<native_tray_icon_t>(tray_icon_raw);
  } catch (...) {
    return nullptr;
  }
}

void native_tray_icon_destroy(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return;

  // Remove event listeners first
  auto listeners_it = g_tray_icon_listeners.find(tray_icon);
  if (listeners_it != g_tray_icon_listeners.end()) {
    g_tray_icon_listeners.erase(listeners_it);
  }

  // Delete TrayIcon instance
  auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
  delete tray_icon_ptr;
}

native_tray_icon_id_t native_tray_icon_get_id(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return -1;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return -1;
    return tray_icon_ptr->GetId();
  } catch (...) {
    return -1;
  }
}

void native_tray_icon_set_icon(native_tray_icon_t tray_icon, native_image_t image) {
  if (!tray_icon)
    return;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return;
    if (image) {
      // Extract the shared_ptr from the native_image_t handle
      auto image_ptr = static_cast<std::shared_ptr<Image>*>(image);
      tray_icon_ptr->SetIcon(*image_ptr);
    } else {
      tray_icon_ptr->SetIcon(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

native_image_t native_tray_icon_get_icon(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return nullptr;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return nullptr;
    auto image = tray_icon_ptr->GetIcon();

    if (!image) {
      return nullptr;
    }

    // Create a new shared_ptr wrapper for the C API
    return new std::shared_ptr<Image>(image);
  } catch (...) {
    return nullptr;
  }
}

void native_tray_icon_set_title(native_tray_icon_t tray_icon, const char* title) {
  if (!tray_icon)
    return;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return;
    if (title) {
      tray_icon_ptr->SetTitle(std::string(title));
    } else {
      tray_icon_ptr->SetTitle(std::nullopt);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

char* native_tray_icon_get_title(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return nullptr;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return nullptr;
    auto title = tray_icon_ptr->GetTitle();
    if (title.has_value()) {
      return to_c_str(title.value());
    } else {
      return nullptr;
    }
  } catch (...) {
    return nullptr;
  }
}

void native_tray_icon_set_tooltip(native_tray_icon_t tray_icon, const char* tooltip) {
  if (!tray_icon)
    return;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return;
    if (tooltip) {
      tray_icon_ptr->SetTooltip(std::string(tooltip));
    } else {
      tray_icon_ptr->SetTooltip(std::nullopt);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

char* native_tray_icon_get_tooltip(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return nullptr;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return nullptr;
    auto tooltip = tray_icon_ptr->GetTooltip();
    if (tooltip.has_value()) {
      return to_c_str(tooltip.value());
    } else {
      return nullptr;
    }
  } catch (...) {
    return nullptr;
  }
}

void native_tray_icon_set_context_menu(native_tray_icon_t tray_icon, native_menu_t menu) {
  if (!tray_icon)
    return;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return;
    if (menu) {
      // IMPORTANT: Do NOT create an owning shared_ptr from a raw pointer here.
      // Menu objects are owned by the global registry in menu_c.cpp and by
      // Swift-side references. Creating an owning shared_ptr would introduce a
      // separate control block and cause double-deletion during shutdown. Use a
      // non-owning aliasing shared_ptr with an empty deleter instead.
      auto menu_raw = static_cast<Menu*>(menu);
      auto menu_ptr = std::shared_ptr<Menu>(menu_raw, [](Menu*) {});
      tray_icon_ptr->SetContextMenu(menu_ptr);
    } else {
      tray_icon_ptr->SetContextMenu(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_t native_tray_icon_get_context_menu(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return nullptr;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return nullptr;
    auto menu = tray_icon_ptr->GetContextMenu();
    return menu ? static_cast<native_menu_t>(menu.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

void native_tray_icon_set_context_menu_trigger(native_tray_icon_t tray_icon,
                                               native_context_menu_trigger_t trigger) {
  if (!tray_icon)
    return;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return;

    // Convert C enum to C++ enum
    ContextMenuTrigger cpp_trigger;
    switch (trigger) {
      case NATIVE_CONTEXT_MENU_TRIGGER_NONE:
        cpp_trigger = ContextMenuTrigger::None;
        break;
      case NATIVE_CONTEXT_MENU_TRIGGER_CLICKED:
        cpp_trigger = ContextMenuTrigger::Clicked;
        break;
      case NATIVE_CONTEXT_MENU_TRIGGER_RIGHT_CLICKED:
        cpp_trigger = ContextMenuTrigger::RightClicked;
        break;
      case NATIVE_CONTEXT_MENU_TRIGGER_DOUBLE_CLICKED:
        cpp_trigger = ContextMenuTrigger::DoubleClicked;
        break;
      default:
        cpp_trigger = ContextMenuTrigger::None;
        break;
    }

    tray_icon_ptr->SetContextMenuTrigger(cpp_trigger);
  } catch (...) {
    // Ignore exceptions
  }
}

native_context_menu_trigger_t native_tray_icon_get_context_menu_trigger(
    native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return NATIVE_CONTEXT_MENU_TRIGGER_NONE;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return NATIVE_CONTEXT_MENU_TRIGGER_NONE;
    ContextMenuTrigger cpp_trigger = tray_icon_ptr->GetContextMenuTrigger();

    // Convert C++ enum to C enum
    switch (cpp_trigger) {
      case ContextMenuTrigger::None:
        return NATIVE_CONTEXT_MENU_TRIGGER_NONE;
      case ContextMenuTrigger::Clicked:
        return NATIVE_CONTEXT_MENU_TRIGGER_CLICKED;
      case ContextMenuTrigger::RightClicked:
        return NATIVE_CONTEXT_MENU_TRIGGER_RIGHT_CLICKED;
      case ContextMenuTrigger::DoubleClicked:
        return NATIVE_CONTEXT_MENU_TRIGGER_DOUBLE_CLICKED;
      default:
        return NATIVE_CONTEXT_MENU_TRIGGER_NONE;
    }
  } catch (...) {
    return NATIVE_CONTEXT_MENU_TRIGGER_NONE;
  }
}

bool native_tray_icon_get_bounds(native_tray_icon_t tray_icon, native_rectangle_t* bounds) {
  if (!tray_icon || !bounds)
    return false;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return false;
    Rectangle cpp_bounds = tray_icon_ptr->GetBounds();

    bounds->x = cpp_bounds.x;
    bounds->y = cpp_bounds.y;
    bounds->width = cpp_bounds.width;
    bounds->height = cpp_bounds.height;

    return true;
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_set_visible(native_tray_icon_t tray_icon, bool visible) {
  if (!tray_icon)
    return false;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return false;
    return tray_icon_ptr->SetVisible(visible);
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_is_visible(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return false;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return false;
    return tray_icon_ptr->IsVisible();
  } catch (...) {
    return false;
  }
}

int native_tray_icon_add_listener(native_tray_icon_t tray_icon,
                                  native_tray_icon_event_type_t event_type,
                                  native_tray_icon_event_callback_t callback,
                                  void* user_data) {
  if (!tray_icon || !callback)
    return -1;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return -1;

    // Create listener data
    auto listener_data = std::make_shared<TrayIconListenerData>();
    listener_data->event_type = event_type;
    listener_data->callback = callback;
    listener_data->user_data = user_data;
    listener_data->listener_id = g_tray_icon_next_listener_id++;

    // Get or create listener list for this tray icon
    auto& listeners = g_tray_icon_listeners[tray_icon];

    // Add event listener based on type
    size_t cpp_listener_id = 0;
    switch (event_type) {
      case NATIVE_TRAY_ICON_EVENT_CLICKED:
        cpp_listener_id = tray_icon_ptr->AddListener<TrayIconClickedEvent>(
            [listener_data](const TrayIconClickedEvent& event) {
              if (listener_data && listener_data->callback) {
                native_tray_icon_clicked_event_t c_event;
                c_event.tray_icon_id = event.GetTrayIconId();
                listener_data->callback(&c_event, listener_data->user_data);
              }
            });
        break;

      case NATIVE_TRAY_ICON_EVENT_RIGHT_CLICKED:
        cpp_listener_id = tray_icon_ptr->AddListener<TrayIconRightClickedEvent>(
            [listener_data](const TrayIconRightClickedEvent& event) {
              if (listener_data && listener_data->callback) {
                native_tray_icon_right_clicked_event_t c_event;
                c_event.tray_icon_id = event.GetTrayIconId();
                listener_data->callback(&c_event, listener_data->user_data);
              }
            });
        break;

      case NATIVE_TRAY_ICON_EVENT_DOUBLE_CLICKED:
        cpp_listener_id = tray_icon_ptr->AddListener<TrayIconDoubleClickedEvent>(
            [listener_data](const TrayIconDoubleClickedEvent& event) {
              if (listener_data && listener_data->callback) {
                native_tray_icon_double_clicked_event_t c_event;
                c_event.tray_icon_id = event.GetTrayIconId();
                listener_data->callback(&c_event, listener_data->user_data);
              }
            });
        break;

      default:
        return -1;
    }

    // Store the C++ listener ID in our data structure
    listener_data->listener_id = cpp_listener_id;
    listeners.push_back(listener_data);

    return static_cast<int>(cpp_listener_id);
  } catch (...) {
    return -1;
  }
}

bool native_tray_icon_remove_listener(native_tray_icon_t tray_icon, int listener_id) {
  if (!tray_icon)
    return false;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return false;

    // Find and remove the listener
    auto it = g_tray_icon_listeners.find(tray_icon);
    if (it != g_tray_icon_listeners.end()) {
      auto& listeners = it->second;
      for (auto lit = listeners.begin(); lit != listeners.end(); ++lit) {
        if (static_cast<int>((*lit)->listener_id) == listener_id) {
          tray_icon_ptr->RemoveListener((*lit)->listener_id);
          listeners.erase(lit);
          return true;
        }
      }
    }
    return false;
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_open_context_menu(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return false;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return false;
    return tray_icon_ptr->OpenContextMenu();
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_close_context_menu(native_tray_icon_t tray_icon) {
  if (!tray_icon)
    return false;

  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (!tray_icon_ptr)
      return false;
    return tray_icon_ptr->CloseContextMenu();
  } catch (...) {
    return false;
  }
}
