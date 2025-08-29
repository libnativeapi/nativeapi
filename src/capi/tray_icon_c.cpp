#include "tray_icon_c.h"
#include "../tray_icon.h"
#include <cstring>
#include <map>
#include <memory>

using namespace nativeapi;

// Internal structure to manage C callbacks
struct TrayIconCallbackData {
  native_tray_icon_left_click_callback_t left_click_callback;
  native_tray_icon_right_click_callback_t right_click_callback;
  native_tray_icon_double_click_callback_t double_click_callback;
  void* left_click_user_data;
  void* right_click_user_data;
  void* double_click_user_data;
};

// Global map to store callback data
static std::map<native_tray_icon_t, std::shared_ptr<TrayIconCallbackData>> g_tray_icon_callbacks;

// TrayIcon C API Implementation

native_tray_icon_t native_tray_icon_create(void) {
  try {
    auto tray_icon = std::make_shared<TrayIcon>();
    return static_cast<native_tray_icon_t>(tray_icon.get());
  } catch (...) {
    return nullptr;
  }
}

native_tray_icon_t native_tray_icon_create_from_native(void* native_tray) {
  if (!native_tray) return nullptr;
  
  try {
    auto tray_icon = std::make_shared<TrayIcon>(native_tray);
    return static_cast<native_tray_icon_t>(tray_icon.get());
  } catch (...) {
    return nullptr;
  }
}

void native_tray_icon_destroy(native_tray_icon_t tray_icon) {
  if (!tray_icon) return;
  
  // Clean up callbacks
  auto it = g_tray_icon_callbacks.find(tray_icon);
  if (it != g_tray_icon_callbacks.end()) {
    g_tray_icon_callbacks.erase(it);
  }
  
  // Note: The actual TrayIcon object is managed by shared_ptr
  // This just removes our reference to it
}

native_tray_icon_id_t native_tray_icon_get_id(native_tray_icon_t tray_icon) {
  if (!tray_icon) return -1;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    return tray_icon_ptr->id;
  } catch (...) {
    return -1;
  }
}

void native_tray_icon_set_icon(native_tray_icon_t tray_icon, const char* icon) {
  if (!tray_icon || !icon) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    tray_icon_ptr->SetIcon(icon);
  } catch (...) {
    // Ignore exceptions
  }
}

void native_tray_icon_set_title(native_tray_icon_t tray_icon, const char* title) {
  if (!tray_icon || !title) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    tray_icon_ptr->SetTitle(title);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_tray_icon_get_title(native_tray_icon_t tray_icon, char* buffer, size_t buffer_size) {
  if (!tray_icon || !buffer || buffer_size == 0) return -1;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    std::string title = tray_icon_ptr->GetTitle();
    
    if (title.length() >= buffer_size) {
      return -1;
    }
    
    strncpy(buffer, title.c_str(), buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return static_cast<int>(title.length());
  } catch (...) {
    return -1;
  }
}

void native_tray_icon_set_tooltip(native_tray_icon_t tray_icon, const char* tooltip) {
  if (!tray_icon || !tooltip) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    tray_icon_ptr->SetTooltip(tooltip);
  } catch (...) {
    // Ignore exceptions
  }
}

int native_tray_icon_get_tooltip(native_tray_icon_t tray_icon, char* buffer, size_t buffer_size) {
  if (!tray_icon || !buffer || buffer_size == 0) return -1;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    std::string tooltip = tray_icon_ptr->GetTooltip();
    
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

void native_tray_icon_set_context_menu(native_tray_icon_t tray_icon, native_menu_t menu) {
  if (!tray_icon) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    if (menu) {
      auto menu_ptr = std::shared_ptr<Menu>(static_cast<Menu*>(menu));
      tray_icon_ptr->SetContextMenu(menu_ptr);
    } else {
      tray_icon_ptr->SetContextMenu(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

native_menu_t native_tray_icon_get_context_menu(native_tray_icon_t tray_icon) {
  if (!tray_icon) return nullptr;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    auto menu = tray_icon_ptr->GetContextMenu();
    return menu ? static_cast<native_menu_t>(menu.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

bool native_tray_icon_get_bounds(native_tray_icon_t tray_icon, native_rectangle_t* bounds) {
  if (!tray_icon || !bounds) return false;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
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

bool native_tray_icon_show(native_tray_icon_t tray_icon) {
  if (!tray_icon) return false;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    return tray_icon_ptr->Show();
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_hide(native_tray_icon_t tray_icon) {
  if (!tray_icon) return false;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    return tray_icon_ptr->Hide();
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_is_visible(native_tray_icon_t tray_icon) {
  if (!tray_icon) return false;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    return tray_icon_ptr->IsVisible();
  } catch (...) {
    return false;
  }
}

void native_tray_icon_set_on_left_click(native_tray_icon_t tray_icon, native_tray_icon_left_click_callback_t callback, void* user_data) {
  if (!tray_icon) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    
    // Get or create callback data
    auto it = g_tray_icon_callbacks.find(tray_icon);
    if (it == g_tray_icon_callbacks.end()) {
      g_tray_icon_callbacks[tray_icon] = std::make_shared<TrayIconCallbackData>();
    }
    
    auto callback_data = g_tray_icon_callbacks[tray_icon];
    callback_data->left_click_callback = callback;
    callback_data->left_click_user_data = user_data;
    
    if (callback) {
      tray_icon_ptr->SetOnLeftClick([callback_data]() {
        if (callback_data && callback_data->left_click_callback) {
          callback_data->left_click_callback(callback_data->left_click_user_data);
        }
      });
    } else {
      tray_icon_ptr->SetOnLeftClick(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

void native_tray_icon_set_on_right_click(native_tray_icon_t tray_icon, native_tray_icon_right_click_callback_t callback, void* user_data) {
  if (!tray_icon) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    
    // Get or create callback data
    auto it = g_tray_icon_callbacks.find(tray_icon);
    if (it == g_tray_icon_callbacks.end()) {
      g_tray_icon_callbacks[tray_icon] = std::make_shared<TrayIconCallbackData>();
    }
    
    auto callback_data = g_tray_icon_callbacks[tray_icon];
    callback_data->right_click_callback = callback;
    callback_data->right_click_user_data = user_data;
    
    if (callback) {
      tray_icon_ptr->SetOnRightClick([callback_data]() {
        if (callback_data && callback_data->right_click_callback) {
          callback_data->right_click_callback(callback_data->right_click_user_data);
        }
      });
    } else {
      tray_icon_ptr->SetOnRightClick(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

void native_tray_icon_set_on_double_click(native_tray_icon_t tray_icon, native_tray_icon_double_click_callback_t callback, void* user_data) {
  if (!tray_icon) return;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    
    // Get or create callback data
    auto it = g_tray_icon_callbacks.find(tray_icon);
    if (it == g_tray_icon_callbacks.end()) {
      g_tray_icon_callbacks[tray_icon] = std::make_shared<TrayIconCallbackData>();
    }
    
    auto callback_data = g_tray_icon_callbacks[tray_icon];
    callback_data->double_click_callback = callback;
    callback_data->double_click_user_data = user_data;
    
    if (callback) {
      tray_icon_ptr->SetOnDoubleClick([callback_data]() {
        if (callback_data && callback_data->double_click_callback) {
          callback_data->double_click_callback(callback_data->double_click_user_data);
        }
      });
    } else {
      tray_icon_ptr->SetOnDoubleClick(nullptr);
    }
  } catch (...) {
    // Ignore exceptions
  }
}

bool native_tray_icon_show_context_menu(native_tray_icon_t tray_icon, double x, double y) {
  if (!tray_icon) return false;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    return tray_icon_ptr->ShowContextMenu(x, y);
  } catch (...) {
    return false;
  }
}

bool native_tray_icon_show_context_menu_default(native_tray_icon_t tray_icon) {
  if (!tray_icon) return false;
  
  try {
    auto tray_icon_ptr = static_cast<TrayIcon*>(tray_icon);
    return tray_icon_ptr->ShowContextMenu();
  } catch (...) {
    return false;
  }
}