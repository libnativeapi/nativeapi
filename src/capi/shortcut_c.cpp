#include "shortcut_c.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include "../shortcut.h"

using namespace nativeapi;

// Helper to get Shortcut pointer from handle
static Shortcut* GetShortcut(native_shortcut_t shortcut) {
  return static_cast<Shortcut*>(shortcut);
}

// Thread-local storage for string returns
static std::mutex g_string_cache_mutex;
static std::unordered_map<native_shortcut_id_t, std::string> g_accelerator_cache;
static std::unordered_map<native_shortcut_id_t, std::string> g_description_cache;

native_shortcut_id_t native_shortcut_get_id(native_shortcut_t shortcut) {
  if (!shortcut)
    return 0;
  return GetShortcut(shortcut)->GetId();
}

const char* native_shortcut_get_accelerator(native_shortcut_t shortcut) {
  if (!shortcut)
    return "";

  auto* sc = GetShortcut(shortcut);
  auto id = sc->GetId();
  auto accelerator = sc->GetAccelerator();

  std::lock_guard<std::mutex> lock(g_string_cache_mutex);
  g_accelerator_cache[id] = accelerator;
  return g_accelerator_cache[id].c_str();
}

const char* native_shortcut_get_description(native_shortcut_t shortcut) {
  if (!shortcut)
    return "";

  auto* sc = GetShortcut(shortcut);
  auto id = sc->GetId();
  auto description = sc->GetDescription();

  std::lock_guard<std::mutex> lock(g_string_cache_mutex);
  g_description_cache[id] = description;
  return g_description_cache[id].c_str();
}

void native_shortcut_set_description(native_shortcut_t shortcut, const char* description) {
  if (!shortcut || !description)
    return;
  GetShortcut(shortcut)->SetDescription(description);
}

native_shortcut_scope_t native_shortcut_get_scope(native_shortcut_t shortcut) {
  if (!shortcut)
    return NATIVE_SHORTCUT_SCOPE_GLOBAL;

  ShortcutScope scope = GetShortcut(shortcut)->GetScope();
  return scope == ShortcutScope::Global ? NATIVE_SHORTCUT_SCOPE_GLOBAL
                                        : NATIVE_SHORTCUT_SCOPE_APPLICATION;
}

void native_shortcut_set_enabled(native_shortcut_t shortcut, bool enabled) {
  if (!shortcut)
    return;
  GetShortcut(shortcut)->SetEnabled(enabled);
}

bool native_shortcut_is_enabled(native_shortcut_t shortcut) {
  if (!shortcut)
    return false;
  return GetShortcut(shortcut)->IsEnabled();
}

void native_shortcut_invoke(native_shortcut_t shortcut) {
  if (!shortcut)
    return;
  GetShortcut(shortcut)->Invoke();
}
