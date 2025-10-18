#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

namespace nativeapi {

/**
 * @brief Thread-safe registry for managing shared objects
 *
 * Registry provides a singleton-based registry for storing and retrieving
 * shared objects of type T using void* keys. The registry is fully thread-safe
 * and supports registration, unregistration, lookup, and bulk operations.
 *
 * @tparam T The type of objects to store in the registry
 *
 * Key features:
 * - Thread-safe operations using mutex synchronization
 * - Singleton pattern with heap allocation to avoid destruction order issues
 * - Support for duplicate detection during registration
 * - Snapshot functionality for thread-safe bulk access
 * - Backward compatibility with legacy interface
 */
template <typename T>
class Registry {
 public:
  using Key = void*;
  using Ptr = std::shared_ptr<T>;

  /**
   * @brief Get the singleton instance of Registry
   *
   * Returns the unique instance of Registry for type T using heap
   * allocation to prevent destruction order issues at program exit.
   *
   * @return Reference to the singleton Registry instance
   * @thread_safety This method is thread-safe
   */
  static Registry& GetInstance() {
    static auto* instance = new Registry();
    return *instance;
  }

  /**
   * @brief Register an object with the given key
   *
   * Attempts to register the provided object with the specified key.
   * If a key already exists, the registration will fail and return false.
   *
   * @param key The key to associate with the object
   * @param object The shared pointer to the object to register
   * @return true if registration succeeded, false if key already exists
   * @thread_safety This method is thread-safe
   */
  bool Register(Key key, Ptr object) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto result = registry_.emplace(key, std::move(object));
    return result.second;
  }

  /**
   * @brief Unregister an object by its key
   *
   * Removes the object associated with the given key from the registry.
   *
   * @param key The key of the object to unregister
   * @return true if an object was found and removed, false if key not found
   * @thread_safety This method is thread-safe
   */
  bool Unregister(Key key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return registry_.erase(key) > 0;
  }

  /**
   * @brief Retrieve an object by its key
   *
   * Looks up and returns the object associated with the given key.
   *
   * @param key The key of the object to retrieve
   * @return Shared pointer to the object if found, nullptr otherwise
   * @thread_safety This method is thread-safe
   */
  Ptr Get(Key key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = registry_.find(key);
    return it != registry_.end() ? it->second : nullptr;
  }

  /**
   * @brief Check if a key exists in the registry
   *
   * Determines whether an object is registered with the given key.
   *
   * @param key The key to check for existence
   * @return true if the key exists, false otherwise
   * @thread_safety This method is thread-safe
   */
  bool Contains(Key key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return registry_.find(key) != registry_.end();
  }

  /**
   * @brief Clear all registered objects
   *
   * Removes all objects from the registry, effectively clearing it.
   *
   * @thread_safety This method is thread-safe
   */
  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    registry_.clear();
  }

  /**
   * @brief Get a thread-safe snapshot of all registered objects
   *
   * Returns a copy of the entire registry contents at the time of the call.
   * This provides a consistent view of the registry state without holding
   * the lock for extended periods.
   *
   * @return Copy of the registry map containing all key-object pairs
   * @thread_safety This method is thread-safe
   */
  std::unordered_map<Key, Ptr> GetSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return registry_;
  }

 private:
  /**
   * @brief Private constructor to enforce singleton pattern
   */
  Registry() = default;

  /**
   * @brief Private destructor to enforce singleton pattern
   */
  ~Registry() = default;

  // Prevent copy construction and assignment to maintain singleton property
  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;
  Registry(Registry&&) = delete;
  Registry& operator=(Registry&&) = delete;

  /**
   * @brief Mutex for thread-safe access to the registry
   */
  mutable std::mutex mutex_;

  /**
   * @brief Internal storage for registered objects
   */
  std::unordered_map<Key, Ptr> registry_;
};

/**
 * @brief Convenience function for accessing the global Registry instance
 *
 * Provides a concise way to access the singleton Registry instance for type T.
 * This function template eliminates the need to call GetInstance() directly.
 *
 * @tparam T The type to store in the registry
 * @return Reference to the global Registry instance for type T
 *
 * @example
 * ```cpp
 * // Register an object
 * auto menu = std::make_shared<Menu>();
 * void* key = menu.get();
 * GlobalRegistry<Menu>().Register(key, menu);
 *
 * // Retrieve an object
 * auto retrieved = GlobalRegistry<Menu>().Get(key);
 * if (retrieved) {
 *     // Use the retrieved menu
 * }
 *
 * // Check if key exists
 * if (GlobalRegistry<Menu>().Contains(key)) {
 *     // Key exists in registry
 * }
 *
 * // Unregister an object
 * GlobalRegistry<Menu>().Unregister(key);
 *
 * // Get all registered objects as a snapshot
 * auto snapshot = GlobalRegistry<Menu>().GetSnapshot();
 * for (const auto& [key, obj] : snapshot) {
 *     // Process each registered object
 * }
 * ```
 */
template <typename T>
inline Registry<T>& GlobalRegistry() {
  return Registry<T>::GetInstance();
}
}  // namespace nativeapi
