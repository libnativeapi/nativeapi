#pragma once

#include <functional>
#include <memory>
#include <string>

#include "foundation/id_allocator.h"

namespace nativeapi {

typedef IdAllocator::IdType ShortcutId;

/**
 * @brief Defines the scope of a keyboard shortcut.
 *
 * This enum specifies whether a shortcut is active globally (system-wide)
 * or only when the application has focus (application-local).
 */
enum class ShortcutScope {
  /**
   * @brief Global shortcut that works system-wide.
   *
   * The shortcut will be triggered regardless of which application has focus.
   * This requires appropriate system permissions on some platforms.
   */
  Global,

  /**
   * @brief Application-local shortcut.
   *
   * The shortcut will only be triggered when the application has focus.
   * This is less intrusive and doesn't require special permissions.
   */
  Application
};

/**
 * @brief Configuration options for creating a keyboard shortcut.
 *
 * This structure contains all the parameters needed to register a new
 * keyboard shortcut, including the key combination, callback function,
 * and optional metadata.
 */
struct ShortcutOptions {
  /**
   * @brief The keyboard shortcut string (e.g., "Ctrl+Shift+A").
   *
   * Follows Electron-style accelerator format:
   * - Modifiers: Ctrl, Alt, Shift, Cmd (macOS), Super (Linux), Meta
   * - Keys: A-Z, 0-9, F1-F12, Space, Tab, Enter, Escape, etc.
   * - Examples: "Ctrl+C", "Cmd+Shift+4", "Alt+F4"
   */
  std::string accelerator;

  /**
   * @brief Function to call when the shortcut is activated.
   *
   * This callback will be invoked on the main thread when the user
   * presses the registered key combination.
   */
  std::function<void()> callback;

  /**
   * @brief Optional human-readable description of the shortcut's purpose.
   *
   * This can be used for displaying shortcut lists or help documentation.
   */
  std::string description;

  /**
   * @brief The scope of the shortcut (global or application-local).
   *
   * Defaults to Global for system-wide shortcuts.
   */
  ShortcutScope scope = ShortcutScope::Global;

  /**
   * @brief Whether the shortcut is initially enabled.
   *
   * Defaults to true. Disabled shortcuts remain registered but won't
   * trigger their callbacks until enabled.
   */
  bool enabled = true;
};

/**
 * @brief Shortcut represents a registered keyboard shortcut.
 *
 * This class encapsulates a keyboard shortcut registration, including
 * its key combination, callback function, and metadata. Shortcut instances
 * are created and managed by the ShortcutManager.
 *
 * Key features:
 * - Unique identifier for each shortcut
 * - Enable/disable functionality without unregistering
 * - Access to shortcut metadata (accelerator, description, scope)
 * - Callback function management
 *
 * @note Shortcut instances should be created through ShortcutManager::Register()
 *       rather than directly constructed.
 * @note This class is not thread-safe. All operations should be performed
 *       on the main thread or properly synchronized.
 *
 * @example
 * ```cpp
 * auto& manager = ShortcutManager::GetInstance();
 * auto shortcut = manager.Register("Ctrl+Shift+Q", []() {
 *     std::cout << "Quick action triggered!" << std::endl;
 * });
 *
 * // Temporarily disable
 * shortcut->SetEnabled(false);
 *
 * // Check properties
 * std::cout << "Accelerator: " << shortcut->GetAccelerator() << std::endl;
 * std::cout << "Scope: " << (shortcut->GetScope() == ShortcutScope::Global ? "Global" :
 * "Application") << std::endl;
 *
 * // Re-enable
 * shortcut->SetEnabled(true);
 * ```
 */
class Shortcut {
 public:
  /**
   * @brief Constructor for creating a shortcut with detailed options.
   *
   * Creates a new shortcut instance with the specified configuration.
   * This constructor is typically called by ShortcutManager::Register().
   *
   * @param id Unique identifier for this shortcut
   * @param options Configuration options for the shortcut
   */
  Shortcut(ShortcutId id, const ShortcutOptions& options);

  /**
   * @brief Constructor for creating a shortcut with basic parameters.
   *
   * Creates a new global shortcut with the specified accelerator and callback.
   * This is a convenience constructor for simple use cases.
   *
   * @param id Unique identifier for this shortcut
   * @param accelerator The keyboard shortcut string
   * @param callback Function to call when activated
   */
  Shortcut(ShortcutId id, const std::string& accelerator, std::function<void()> callback);

  /**
   * @brief Virtual destructor for proper cleanup.
   *
   * Ensures proper cleanup of resources when the shortcut is destroyed.
   */
  virtual ~Shortcut();

  /**
   * @brief Get the unique identifier of this shortcut.
   *
   * @return The shortcut's unique ID
   */
  ShortcutId GetId() const;

  /**
   * @brief Get the keyboard accelerator string.
   *
   * Returns the key combination string used to trigger this shortcut,
   * such as "Ctrl+Shift+A" or "Cmd+Space".
   *
   * @return The accelerator string
   */
  std::string GetAccelerator() const;

  /**
   * @brief Get the human-readable description of this shortcut.
   *
   * Returns the description provided when the shortcut was created,
   * or an empty string if no description was set.
   *
   * @return The shortcut description
   */
  std::string GetDescription() const;

  /**
   * @brief Set a new description for this shortcut.
   *
   * Updates the human-readable description. This doesn't affect the
   * shortcut's functionality, only its metadata.
   *
   * @param description The new description text
   */
  void SetDescription(const std::string& description);

  /**
   * @brief Get the scope of this shortcut.
   *
   * Returns whether this is a global (system-wide) or application-local
   * shortcut.
   *
   * @return The shortcut scope
   */
  ShortcutScope GetScope() const;

  /**
   * @brief Enable or disable this shortcut.
   *
   * When disabled, the shortcut remains registered but won't trigger
   * its callback. This is useful for temporarily disabling shortcuts
   * without unregistering them.
   *
   * @param enabled true to enable, false to disable
   *
   * @example
   * ```cpp
   * // Disable during modal dialog
   * shortcut->SetEnabled(false);
   * ShowModalDialog();
   * shortcut->SetEnabled(true);
   * ```
   */
  void SetEnabled(bool enabled);

  /**
   * @brief Check if this shortcut is currently enabled.
   *
   * @return true if enabled, false if disabled
   */
  bool IsEnabled() const;

  /**
   * @brief Invoke the shortcut's callback function.
   *
   * Manually triggers the shortcut's callback. This is primarily used
   * internally by the ShortcutManager when the key combination is pressed,
   * but can also be called programmatically for testing or automation.
   *
   * @note This method respects the enabled state - it won't invoke the
   *       callback if the shortcut is disabled.
   *
   * @example
   * ```cpp
   * // Programmatically trigger shortcut
   * if (shortcut->IsEnabled()) {
   *     shortcut->Invoke();
   * }
   * ```
   */
  void Invoke();

  /**
   * @brief Set a new callback function for this shortcut.
   *
   * Replaces the current callback with a new one. This allows changing
   * the shortcut's behavior without unregistering and re-registering it.
   *
   * @param callback The new callback function
   *
   * @example
   * ```cpp
   * // Change behavior dynamically
   * shortcut->SetCallback([]() {
   *     std::cout << "New behavior!" << std::endl;
   * });
   * ```
   */
  void SetCallback(std::function<void()> callback);

  /**
   * @brief Get the current callback function.
   *
   * Returns a copy of the callback function. This is primarily useful
   * for testing or introspection.
   *
   * @return The callback function
   */
  std::function<void()> GetCallback() const;

 private:
  /**
   * @brief Unique identifier for this shortcut.
   */
  ShortcutId id_;

  /**
   * @brief The keyboard accelerator string (e.g., "Ctrl+Shift+A").
   */
  std::string accelerator_;

  /**
   * @brief Human-readable description of the shortcut's purpose.
   */
  std::string description_;

  /**
   * @brief The scope of the shortcut (global or application-local).
   */
  ShortcutScope scope_;

  /**
   * @brief Whether the shortcut is currently enabled.
   */
  bool enabled_;

  /**
   * @brief The callback function to invoke when the shortcut is triggered.
   */
  std::function<void()> callback_;
};

}  // namespace nativeapi
