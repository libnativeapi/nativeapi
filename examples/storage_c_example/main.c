#include <nativeapi.h>
#include <stdio.h>
#include <stdlib.h>

void demo_preferences() {
  printf("=== Preferences C API Demo ===\n");

  // Create preferences with custom scope
  native_preferences_t prefs = native_preferences_create_with_scope("my_c_app");
  if (!prefs) {
    printf("Failed to create preferences\n");
    return;
  }

  // Store some values
  native_preferences_set(prefs, "username", "alice");
  native_preferences_set(prefs, "theme", "light");
  native_preferences_set(prefs, "font_size", "12");

  // Retrieve values
  char* username = native_preferences_get(prefs, "username", "");
  char* theme = native_preferences_get(prefs, "theme", "");
  char* font_size = native_preferences_get(prefs, "font_size", "");

  printf("Username: %s\n", username);
  printf("Theme: %s\n", theme);
  printf("Font size: %s\n", font_size);

  native_preferences_free_string(username);
  native_preferences_free_string(theme);
  native_preferences_free_string(font_size);

  // Check if key exists
  if (native_preferences_contains(prefs, "language")) {
    char* language = native_preferences_get(prefs, "language", "");
    printf("Language: %s\n", language);
    native_preferences_free_string(language);
  } else {
    char* default_lang = native_preferences_get(prefs, "language", "en");
    printf("Language not set, using default: %s\n", default_lang);
    native_preferences_free_string(default_lang);
  }

  // Get all keys
  char** keys = NULL;
  size_t count = 0;
  if (native_preferences_get_keys(prefs, &keys, &count)) {
    printf("\nAll keys (%zu):\n", count);
    for (size_t i = 0; i < count; i++) {
      char* value = native_preferences_get(prefs, keys[i], "");
      printf("  - %s: %s\n", keys[i], value);
      native_preferences_free_string(value);
    }
    native_preferences_free_string_array(keys, count);
  }

  // Get size
  size_t size = native_preferences_get_size(prefs);
  printf("Total items: %zu\n", size);

  // Remove a key
  printf("\nRemoving 'font_size'...\n");
  native_preferences_remove(prefs, "font_size");
  printf("Size after removal: %zu\n", native_preferences_get_size(prefs));

  // Get scope
  char* scope = native_preferences_get_scope(prefs);
  printf("Scope: %s\n", scope);
  native_preferences_free_string(scope);

  // Clean up
  native_preferences_destroy(prefs);

  printf("\n");
}

void demo_secure_storage() {
  printf("=== Secure Storage C API Demo ===\n");

  // Check if secure storage is available
  if (!native_secure_storage_is_available()) {
    printf("Secure storage is not available on this platform\n");
    return;
  }

  // Create secure storage with custom scope
  native_secure_storage_t storage = native_secure_storage_create_with_scope("my_c_app_secure");
  if (!storage) {
    printf("Failed to create secure storage\n");
    return;
  }

  // Store sensitive data
  native_secure_storage_set(storage, "api_key", "sk-c-api-1234567890");
  native_secure_storage_set(storage, "secret", "my_secret_value");
  native_secure_storage_set(storage, "token", "bearer_token_xyz");

  // Retrieve sensitive data
  char* api_key = native_secure_storage_get(storage, "api_key", "");
  char* secret = native_secure_storage_get(storage, "secret", "");

  printf("API Key: %s\n", api_key);
  printf("Secret: %s\n", secret);

  native_secure_storage_free_string(api_key);
  native_secure_storage_free_string(secret);

  // Get all keys
  char** keys = NULL;
  size_t count = 0;
  if (native_secure_storage_get_keys(storage, &keys, &count)) {
    printf("\nStored secure items (%zu):\n", count);
    for (size_t i = 0; i < count; i++) {
      printf("  - %s: [encrypted]\n", keys[i]);
    }
    native_secure_storage_free_string_array(keys, count);
  }

  // Check existence
  if (native_secure_storage_contains(storage, "api_key")) {
    printf("\nAPI key is securely stored\n");
  }

  // Get size
  size_t size = native_secure_storage_get_size(storage);
  printf("Total secure items: %zu\n", size);

  // Remove sensitive data
  printf("\nRemoving 'token'...\n");
  native_secure_storage_remove(storage, "token");
  printf("Size after removal: %zu\n", native_secure_storage_get_size(storage));

  // Get scope
  char* storage_scope = native_secure_storage_get_scope(storage);
  printf("Scope: %s\n", storage_scope);
  native_secure_storage_free_string(storage_scope);

  // Clean up (optional: clear all for this demo)
  // native_secure_storage_clear(storage);

  native_secure_storage_destroy(storage);

  printf("\n");
}

int main() {
  printf("Storage C API Example\n");
  printf("=====================\n\n");

  // Demo Preferences (plain text storage)
  demo_preferences();

  // Demo SecureStorage (encrypted storage)
  demo_secure_storage();

  printf("Done! Check your system's storage locations:\n");
  printf("  - macOS: NSUserDefaults & Keychain\n");
  printf("  - Windows: Registry & DPAPI\n");
  printf("  - Linux: ~/.config/nativeapi & ~/.local/share/nativeapi\n");

  return 0;
}
