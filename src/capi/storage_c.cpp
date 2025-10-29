#include "storage_c.h"
#include <cstring>
#include <memory>
#include "../preferences.h"
#include "../secure_storage.h"

using namespace nativeapi;

// Helper function to convert std::string to C string
static char* string_to_cstr(const std::string& str) {
  char* result = new char[str.length() + 1];
  std::strcpy(result, str.c_str());
  return result;
}

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
    return default_value ? string_to_cstr(default_value) : string_to_cstr("");
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    std::string result = preferences->Get(key, default_value ? default_value : "");
    return string_to_cstr(result);
  } catch (...) {
    return default_value ? string_to_cstr(default_value) : string_to_cstr("");
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
      (*out_keys)[i] = string_to_cstr(keys[i]);
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
    return string_to_cstr("");
  }

  try {
    auto* preferences = static_cast<Preferences*>(prefs);
    return string_to_cstr(preferences->GetScope());
  } catch (...) {
    return string_to_cstr("");
  }
}

void native_preferences_free_string(char* str) {
  if (str) {
    delete[] str;
  }
}

void native_preferences_free_string_array(char** strings, size_t count) {
  if (strings) {
    for (size_t i = 0; i < count; ++i) {
      delete[] strings[i];
    }
    delete[] strings;
  }
}

// SecureStorage C API Implementation

native_secure_storage_t native_secure_storage_create(void) {
  try {
    auto* storage = new SecureStorage();
    return static_cast<void*>(storage);
  } catch (...) {
    return nullptr;
  }
}

native_secure_storage_t native_secure_storage_create_with_scope(const char* scope) {
  if (!scope) {
    return nullptr;
  }

  try {
    auto* storage = new SecureStorage(scope);
    return static_cast<void*>(storage);
  } catch (...) {
    return nullptr;
  }
}

void native_secure_storage_destroy(native_secure_storage_t storage) {
  if (storage) {
    delete static_cast<SecureStorage*>(storage);
  }
}

bool native_secure_storage_set(native_secure_storage_t storage,
                               const char* key,
                               const char* value) {
  if (!storage || !key || !value) {
    return false;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return secure_storage->Set(key, value);
  } catch (...) {
    return false;
  }
}

char* native_secure_storage_get(native_secure_storage_t storage,
                                const char* key,
                                const char* default_value) {
  if (!storage || !key) {
    return default_value ? string_to_cstr(default_value) : string_to_cstr("");
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    std::string result = secure_storage->Get(key, default_value ? default_value : "");
    return string_to_cstr(result);
  } catch (...) {
    return default_value ? string_to_cstr(default_value) : string_to_cstr("");
  }
}

bool native_secure_storage_remove(native_secure_storage_t storage, const char* key) {
  if (!storage || !key) {
    return false;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return secure_storage->Remove(key);
  } catch (...) {
    return false;
  }
}

bool native_secure_storage_clear(native_secure_storage_t storage) {
  if (!storage) {
    return false;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return secure_storage->Clear();
  } catch (...) {
    return false;
  }
}

bool native_secure_storage_contains(native_secure_storage_t storage, const char* key) {
  if (!storage || !key) {
    return false;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return secure_storage->Contains(key);
  } catch (...) {
    return false;
  }
}

bool native_secure_storage_get_keys(native_secure_storage_t storage,
                                    char*** out_keys,
                                    size_t* out_count) {
  if (!storage || !out_keys || !out_count) {
    return false;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    auto keys = secure_storage->GetKeys();

    *out_count = keys.size();
    *out_keys = new char*[keys.size()];

    for (size_t i = 0; i < keys.size(); ++i) {
      (*out_keys)[i] = string_to_cstr(keys[i]);
    }

    return true;
  } catch (...) {
    *out_keys = nullptr;
    *out_count = 0;
    return false;
  }
}

size_t native_secure_storage_get_size(native_secure_storage_t storage) {
  if (!storage) {
    return 0;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return secure_storage->GetSize();
  } catch (...) {
    return 0;
  }
}

char* native_secure_storage_get_scope(native_secure_storage_t storage) {
  if (!storage) {
    return string_to_cstr("");
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return string_to_cstr(secure_storage->GetScope());
  } catch (...) {
    return string_to_cstr("");
  }
}

bool native_secure_storage_is_available(void) {
  try {
    return SecureStorage::IsAvailable();
  } catch (...) {
    return false;
  }
}

void native_secure_storage_free_string(char* str) {
  if (str) {
    delete[] str;
  }
}

void native_secure_storage_free_string_array(char** strings, size_t count) {
  if (strings) {
    for (size_t i = 0; i < count; ++i) {
      delete[] strings[i];
    }
    delete[] strings;
  }
}
