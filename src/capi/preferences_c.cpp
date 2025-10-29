#include "preferences_c.h"
#include "../preferences.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Preferences C API Implementation

native_preferences_t native_preferences_create(void) {
  try {
    auto* prefs = new Preferences();
    return static_cast<void*>(prefs);
  } catch (...) {
    return nullptr;
  }
}

native_preferences_t native_preferences_create_with_scope(const char* scope) {
  if (!scope) {
    return nullptr;
  }

  try {
    auto* prefs = new Preferences(scope);
    return static_cast<void*>(prefs);
  } catch (...) {
    return nullptr;
  }
}

void native_preferences_destroy(native_preferences_t prefs) {
  if (prefs) {
    delete static_cast<Preferences*>(prefs);
  }
}

bool native_preferences_set(native_preferences_t prefs, const char* key, const char* value) {
  if (!prefs || !key || !value) {
    return false;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return preferences->Set(key, value);
  } catch (...) {
    return false;
  }
}

char* native_preferences_get(native_preferences_t prefs,
                             const char* key,
                             const char* default_value) {
  if (!prefs || !key) {
    return default_value ? to_c_str(std::string(default_value)) : nullptr;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    std::string result = preferences->Get(key, default_value ? default_value : "");
    return to_c_str(result);
  } catch (...) {
    return default_value ? to_c_str(std::string(default_value)) : nullptr;
  }
}

bool native_preferences_remove(native_preferences_t prefs, const char* key) {
  if (!prefs || !key) {
    return false;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return preferences->Remove(key);
  } catch (...) {
    return false;
  }
}

bool native_preferences_clear(native_preferences_t prefs) {
  if (!prefs) {
    return false;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return preferences->Clear();
  } catch (...) {
    return false;
  }
}

bool native_preferences_contains(native_preferences_t prefs, const char* key) {
  if (!prefs || !key) {
    return false;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return preferences->Contains(key);
  } catch (...) {
    return false;
  }
}

bool native_preferences_get_keys(native_preferences_t prefs, char*** out_keys, size_t* out_count) {
  if (!prefs || !out_keys || !out_count) {
    return false;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    auto keys = preferences->GetKeys();

    *out_count = keys.size();
    *out_keys = new char*[keys.size()];

    for (size_t i = 0; i < keys.size(); ++i) {
      (*out_keys)[i] = to_c_str(keys[i]);
    }

    return true;
  } catch (...) {
    *out_keys = nullptr;
    *out_count = 0;
    return false;
  }
}

size_t native_preferences_get_size(native_preferences_t prefs) {
  if (!prefs) {
    return 0;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return preferences->GetSize();
  } catch (...) {
    return 0;
  }
}

char* native_preferences_get_scope(native_preferences_t prefs) {
  if (!prefs) {
    return nullptr;
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return to_c_str(preferences->GetScope());
  } catch (...) {
    return nullptr;
  }
}

void native_preferences_free_string(char* str) {
  free_c_str(str);
}

void native_preferences_free_string_array(char** strings, size_t count) {
  if (strings) {
    for (size_t i = 0; i < count; ++i) {
      free_c_str(strings[i]);
    }
    delete[] strings;
  }
}
