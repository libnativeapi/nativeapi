#include "secure_storage_c.h"
#include "../secure_storage.h"
#include "string_utils_c.h"

using namespace nativeapi;

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
    return default_value ? to_c_str(std::string(default_value)) : nullptr;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    std::string result = secure_storage->Get(key, default_value ? default_value : "");
    return to_c_str(result);
  } catch (...) {
    return default_value ? to_c_str(std::string(default_value)) : nullptr;
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
      (*out_keys)[i] = to_c_str(keys[i]);
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
    return nullptr;
  }

  try {
    auto* secure_storage = static_cast<SecureStorage*>(storage);
    return to_c_str(secure_storage->GetScope());
  } catch (...) {
    return nullptr;
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
  free_c_str(str);
}

void native_secure_storage_free_string_array(char** strings, size_t count) {
  if (strings) {
    for (size_t i = 0; i < count; ++i) {
      free_c_str(strings[i]);
    }
    delete[] strings;
  }
}
