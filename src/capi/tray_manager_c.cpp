#include "tray_manager_c.h"
#include <memory>
#include "../tray_manager.h"

using namespace nativeapi;

// TrayManager C API Implementation

bool native_tray_manager_is_supported(void) {
  try {
    return TrayManager::GetInstance().IsSupported();
  } catch (...) {
    return false;
  }
}

native_tray_icon_t native_tray_manager_get(native_tray_icon_id_t tray_icon_id) {
  try {
    auto tray_icon = TrayManager::GetInstance().Get(static_cast<TrayIconId>(tray_icon_id));
    return tray_icon ? static_cast<native_tray_icon_t>(tray_icon.get()) : nullptr;
  } catch (...) {
    return nullptr;
  }
}

native_tray_icon_list_t native_tray_manager_get_all(void) {
  native_tray_icon_list_t result = {nullptr, 0};

  try {
    auto tray_icons = TrayManager::GetInstance().GetAll();

    if (tray_icons.empty()) {
      return result;
    }

    result.tray_icons =
        static_cast<native_tray_icon_t*>(malloc(tray_icons.size() * sizeof(native_tray_icon_t)));
    if (!result.tray_icons) {
      return result;
    }

    result.count = tray_icons.size();
    for (size_t i = 0; i < tray_icons.size(); ++i) {
      result.tray_icons[i] = static_cast<native_tray_icon_t>(tray_icons[i].get());
    }

    return result;
  } catch (...) {
    if (result.tray_icons) {
      free(result.tray_icons);
      result.tray_icons = nullptr;
      result.count = 0;
    }
    return result;
  }
}

// Utility functions

void native_tray_icon_list_free(native_tray_icon_list_t list) {
  if (list.tray_icons) {
    free(list.tray_icons);
  }
}