#pragma once

#include <string>

#include "foundation/event.h"
#include "shortcut.h"

namespace nativeapi {

/**
 * @brief Base class for all shortcut-related events.
 *
 * This class provides common functionality for shortcut events,
 * including access to the shortcut ID and accelerator string that
 * triggered the event.
 */
class ShortcutEvent : public Event {
 public:
  /**
   * @brief Constructor for ShortcutEvent.
   *
   * @param shortcut_id The unique ID of the shortcut that triggered this event
   * @param accelerator The keyboard accelerator string (e.g., "Ctrl+Shift+A")
   */
  explicit ShortcutEvent(ShortcutId shortcut_id, const std::string& accelerator)
      : shortcut_id_(shortcut_id), accelerator_(accelerator) {}

  /**
   * @brief Virtual destructor.
   */
  virtual ~ShortcutEvent() = default;

  /**
   * @brief Get the shortcut ID associated with this event.
   *
   * @return The unique identifier of the shortcut
   */
  ShortcutId GetShortcutId() const { return shortcut_id_; }

  /**
   * @brief Get the accelerator string associated with this event.
   *
   * Returns the keyboard shortcut string that was pressed,
   * such as "Ctrl+Shift+A" or "Cmd+Space".
   *
   * @return The accelerator string
   */
  std::string GetAccelerator() const { return accelerator_; }

  /**
   * @brief Get a string representation of the event type (for debugging).
   *
   * Default implementation returns "ShortcutEvent".
   *
   * @return The event type name
   */
  std::string GetTypeName() const override { return "ShortcutEvent"; }

 private:
  /**
   * @brief The unique ID of the shortcut.
   */
  ShortcutId shortcut_id_;

  /**
   * @brief The keyboard accelerator string.
   */
  std::string accelerator_;
};

/**
 * @brief Event emitted when a keyboard shortcut is activated.
 *
 * This event is emitted when a registered keyboard shortcut is triggered
 * by the user pressing the corresponding key combination. The event is
 * emitted before the shortcut's callback is invoked, allowing listeners
 * to perform additional actions or logging.
 *
 * @example
 * ```cpp
 * auto& manager = ShortcutManager::GetInstance();
 *
 * // Listen for shortcut activations
 * manager.AddListener<ShortcutActivatedEvent>([](const ShortcutActivatedEvent& event) {
 *     std::cout << "Shortcut activated: " << event.GetAccelerator() << std::endl;
 *     std::cout << "Shortcut ID: " << event.GetShortcutId() << std::endl;
 * });
 *
 * // Register a shortcut
 * auto shortcut = manager.Register("Ctrl+Shift+Q", []() {
 *     std::cout << "Quick action!" << std::endl;
 * });
 * ```
 */
class ShortcutActivatedEvent : public ShortcutEvent {
 public:
  /**
   * @brief Constructor for ShortcutActivatedEvent.
   *
   * @param shortcut_id The unique ID of the activated shortcut
   * @param accelerator The keyboard accelerator string
   */
  explicit ShortcutActivatedEvent(ShortcutId shortcut_id, const std::string& accelerator)
      : ShortcutEvent(shortcut_id, accelerator) {}

  /**
   * @brief Get a string representation of the event type.
   *
   * @return "ShortcutActivatedEvent"
   */
  std::string GetTypeName() const override { return "ShortcutActivatedEvent"; }
};

/**
 * @brief Event emitted when a keyboard shortcut is successfully registered.
 *
 * This event is emitted by the ShortcutManager when a new shortcut is
 * successfully registered with the system. It allows listeners to track
 * which shortcuts are active and perform any necessary setup.
 *
 * @example
 * ```cpp
 * auto& manager = ShortcutManager::GetInstance();
 *
 * // Listen for shortcut registrations
 * manager.AddListener<ShortcutRegisteredEvent>([](const ShortcutRegisteredEvent& event) {
 *     std::cout << "Shortcut registered: " << event.GetAccelerator() << std::endl;
 *     // Update UI to show available shortcuts
 * });
 * ```
 */
class ShortcutRegisteredEvent : public ShortcutEvent {
 public:
  /**
   * @brief Constructor for ShortcutRegisteredEvent.
   *
   * @param shortcut_id The unique ID of the registered shortcut
   * @param accelerator The keyboard accelerator string
   */
  explicit ShortcutRegisteredEvent(ShortcutId shortcut_id, const std::string& accelerator)
      : ShortcutEvent(shortcut_id, accelerator) {}

  /**
   * @brief Get a string representation of the event type.
   *
   * @return "ShortcutRegisteredEvent"
   */
  std::string GetTypeName() const override { return "ShortcutRegisteredEvent"; }
};

/**
 * @brief Event emitted when a keyboard shortcut is unregistered.
 *
 * This event is emitted by the ShortcutManager when a shortcut is
 * unregistered and removed from the system. It allows listeners to
 * track shortcut lifecycle and perform cleanup.
 *
 * @example
 * ```cpp
 * auto& manager = ShortcutManager::GetInstance();
 *
 * // Listen for shortcut unregistrations
 * manager.AddListener<ShortcutUnregisteredEvent>([](const ShortcutUnregisteredEvent& event) {
 *     std::cout << "Shortcut unregistered: " << event.GetAccelerator() << std::endl;
 *     // Update UI to remove shortcut from list
 * });
 * ```
 */
class ShortcutUnregisteredEvent : public ShortcutEvent {
 public:
  /**
   * @brief Constructor for ShortcutUnregisteredEvent.
   *
   * @param shortcut_id The unique ID of the unregistered shortcut
   * @param accelerator The keyboard accelerator string
   */
  explicit ShortcutUnregisteredEvent(ShortcutId shortcut_id, const std::string& accelerator)
      : ShortcutEvent(shortcut_id, accelerator) {}

  /**
   * @brief Get a string representation of the event type.
   *
   * @return "ShortcutUnregisteredEvent"
   */
  std::string GetTypeName() const override { return "ShortcutUnregisteredEvent"; }
};

/**
 * @brief Event emitted when a shortcut registration fails.
 *
 * This event is emitted when the system fails to register a keyboard
 * shortcut, typically due to conflicts with existing shortcuts or
 * system restrictions. The event includes an error message describing
 * the failure reason.
 *
 * @example
 * ```cpp
 * auto& manager = ShortcutManager::GetInstance();
 *
 * // Listen for registration failures
 * manager.AddListener<ShortcutRegistrationFailedEvent>([](const ShortcutRegistrationFailedEvent& event) {
 *     std::cerr << "Failed to register shortcut: " << event.GetAccelerator() << std::endl;
 *     std::cerr << "Reason: " << event.GetErrorMessage() << std::endl;
 * });
 * ```
 */
class ShortcutRegistrationFailedEvent : public ShortcutEvent {
 public:
  /**
   * @brief Constructor for ShortcutRegistrationFailedEvent.
   *
   * @param shortcut_id The unique ID that was assigned to the shortcut (may be 0 if not assigned)
   * @param accelerator The keyboard accelerator string that failed to register
   * @param error_message Description of why the registration failed
   */
  explicit ShortcutRegistrationFailedEvent(ShortcutId shortcut_id,
                                           const std::string& accelerator,
                                           const std::string& error_message)
      : ShortcutEvent(shortcut_id, accelerator), error_message_(error_message) {}

  /**
   * @brief Get the error message describing why registration failed.
   *
   * @return The error message
   */
  std::string GetErrorMessage() const { return error_message_; }

  /**
   * @brief Get a string representation of the event type.
   *
   * @return "ShortcutRegistrationFailedEvent"
   */
  std::string GetTypeName() const override { return "ShortcutRegistrationFailedEvent"; }

 private:
  /**
   * @brief Description of the registration failure.
   */
  std::string error_message_;
};

}  // namespace nativeapi

