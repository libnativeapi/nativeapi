#include "shortcut_manager.h"

#include <regex>

namespace nativeapi {

// Singleton instance
ShortcutManager& ShortcutManager::GetInstance() {
  static ShortcutManager instance;
  return instance;
}

// Check if global shortcuts are supported
bool ShortcutManager::IsSupported() {
  return pimpl_->IsSupported();
}

// Register a new keyboard shortcut with callback
std::shared_ptr<Shortcut> ShortcutManager::Register(const std::string& accelerator,
                                                    std::function<void()> callback) {
  ShortcutOptions options;
  options.accelerator = accelerator;
  options.callback = callback;
  return Register(options);
}

// Register a new keyboard shortcut with options
std::shared_ptr<Shortcut> ShortcutManager::Register(const ShortcutOptions& options) {
  std::unique_lock<std::mutex> lock(mutex_);

  // Validate accelerator format
  if (!IsValidAccelerator(options.accelerator)) {
    // Emit failure event
    EmitAsync<ShortcutRegistrationFailedEvent>(0, options.accelerator,
                                               "Invalid accelerator format");
    return nullptr;
  }

  // Check if accelerator is already registered
  if (!IsAvailable(options.accelerator)) {
    // Emit failure event
    EmitAsync<ShortcutRegistrationFailedEvent>(0, options.accelerator,
                                               "Accelerator already registered");
    return nullptr;
  }

  // Allocate new ID
  ShortcutId id = next_shortcut_id_++;

  // Create shortcut instance
  auto shortcut = std::make_shared<Shortcut>(id, options);

  // Register with platform
  if (!pimpl_->RegisterShortcut(shortcut)) {
    // Platform registration failed
    EmitAsync<ShortcutRegistrationFailedEvent>(id, options.accelerator,
                                               "Platform registration failed");
    return nullptr;
  }

  // Store in registries
  shortcuts_by_id_[id] = shortcut;
  shortcuts_by_accelerator_[options.accelerator] = shortcut;

  // Emit success event
  EmitAsync<ShortcutRegisteredEvent>(id, options.accelerator);

  return shortcut;
}

// Unregister a shortcut by ID
bool ShortcutManager::Unregister(ShortcutId id) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = shortcuts_by_id_.find(id);
  if (it == shortcuts_by_id_.end()) {
    return false;
  }

  auto shortcut = it->second;
  std::string accelerator = shortcut->GetAccelerator();

  // Unregister from platform
  pimpl_->UnregisterShortcut(shortcut);

  // Remove from registries
  shortcuts_by_id_.erase(it);
  shortcuts_by_accelerator_.erase(accelerator);

  // Emit event
  EmitAsync<ShortcutUnregisteredEvent>(id, accelerator);

  return true;
}

// Unregister a shortcut by accelerator
bool ShortcutManager::Unregister(const std::string& accelerator) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = shortcuts_by_accelerator_.find(accelerator);
  if (it == shortcuts_by_accelerator_.end()) {
    return false;
  }

  ShortcutId id = it->second->GetId();

  lock.unlock();
  bool result = Unregister(id);
  lock.lock();

  return result;
}

// Unregister all shortcuts
int ShortcutManager::UnregisterAll() {
  std::unique_lock<std::mutex> lock(mutex_);

  int count = 0;

  // Create a copy of IDs to avoid iterator invalidation
  std::vector<ShortcutId> ids;
  ids.reserve(shortcuts_by_id_.size());
  for (const auto& [id, shortcut] : shortcuts_by_id_) {
    ids.push_back(id);
  }

  lock.unlock();
  for (ShortcutId id : ids) {
    if (Unregister(id)) {
      count++;
    }
  }
  lock.lock();

  return count;
}

// Get a shortcut by ID
std::shared_ptr<Shortcut> ShortcutManager::Get(ShortcutId id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = shortcuts_by_id_.find(id);
  return (it != shortcuts_by_id_.end()) ? it->second : nullptr;
}

// Get a shortcut by accelerator
std::shared_ptr<Shortcut> ShortcutManager::Get(const std::string& accelerator) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = shortcuts_by_accelerator_.find(accelerator);
  return (it != shortcuts_by_accelerator_.end()) ? it->second : nullptr;
}

// Get all shortcuts
std::vector<std::shared_ptr<Shortcut>> ShortcutManager::GetAll() {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<std::shared_ptr<Shortcut>> result;
  result.reserve(shortcuts_by_id_.size());

  for (const auto& [id, shortcut] : shortcuts_by_id_) {
    result.push_back(shortcut);
  }

  return result;
}

// Get shortcuts by scope
std::vector<std::shared_ptr<Shortcut>> ShortcutManager::GetByScope(ShortcutScope scope) {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<std::shared_ptr<Shortcut>> result;

  for (const auto& [id, shortcut] : shortcuts_by_id_) {
    if (shortcut->GetScope() == scope) {
      result.push_back(shortcut);
    }
  }

  return result;
}

// Check if an accelerator is available
bool ShortcutManager::IsAvailable(const std::string& accelerator) {
  std::lock_guard<std::mutex> lock(mutex_);
  return shortcuts_by_accelerator_.find(accelerator) == shortcuts_by_accelerator_.end();
}

// Validate accelerator format
bool ShortcutManager::IsValidAccelerator(const std::string& accelerator) {
  if (accelerator.empty()) {
    return false;
  }

  // Basic validation using regex
  // Format: [Modifier+]*Key
  // Modifiers: Ctrl, Alt, Shift, Cmd, Super, Meta
  // Keys: A-Z, 0-9, F1-F24, Space, Tab, Enter, Escape, etc.

  std::regex accelerator_regex(
      R"(^(?:(?:Ctrl|Alt|Shift|Cmd|Super|Meta|CmdOrCtrl)\+)*(?:[A-Za-z0-9]|F[1-9]|F1[0-9]|F2[0-4]|Space|Tab|Enter|Escape|Backspace|Delete|Insert|Home|End|PageUp|PageDown|Up|Down|Left|Right|Plus|Minus|Equal)$)",
      std::regex::icase);

  return std::regex_match(accelerator, accelerator_regex);
}

// Enable or disable shortcut processing
void ShortcutManager::SetEnabled(bool enabled) {
  std::lock_guard<std::mutex> lock(mutex_);
  enabled_ = enabled;
}

// Check if shortcut processing is enabled
bool ShortcutManager::IsEnabled() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return enabled_;
}

void ShortcutManager::EmitShortcutActivated(ShortcutId id, const std::string& accelerator) {
  EmitAsync<ShortcutActivatedEvent>(id, accelerator);
}

// Start event listening (called when first listener is added)
void ShortcutManager::StartEventListening() {
  pimpl_->SetupEventMonitoring();
}

// Stop event listening (called when last listener is removed)
void ShortcutManager::StopEventListening() {
  pimpl_->CleanupEventMonitoring();
}

}  // namespace nativeapi
