#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "foundation/event.h"
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "window.h"
#include "window_event.h"

namespace nativeapi {

/**
 * @brief WindowManager is a singleton that manages all windows in the application
 *
 * The WindowManager provides a centralized interface for creating, managing, and
 * monitoring windows across the entire application. It follows the singleton pattern
 * to ensure there's only one instance managing all windows, and provides event
 * notifications for various window state changes.
 *
 * Key features:
 * - Singleton pattern ensures single point of window management
 * - Event-driven architecture for window state notifications
 * - Cross-platform window creation and management
 * - Thread-safe access to the singleton instance
 * - Automatic cleanup of resources on destruction
 *
 * @note This class is thread-safe for singleton access, but individual operations
 *       may require additional synchronization depending on the platform implementation.
 */
class WindowManager : public EventEmitter<WindowEvent> {
 public:
  /**
   * @brief Get the singleton instance of WindowManager
   *
   * This method provides access to the unique instance of WindowManager using
   * the Meyer's singleton pattern. The instance is created on first call and
   * remains alive for the duration of the application. This method is thread-safe
   * and guarantees that only one instance will be created even in multi-threaded
   * environments.
   *
   * @return Reference to the singleton WindowManager instance
   * @thread_safety This method is thread-safe
   *
   * @code
   * // Usage example:
   * auto& manager = WindowManager::GetInstance();
   * auto window = manager.Create(options);
   * @endcode
   */
  static WindowManager& GetInstance();

  /**
   * @brief Destructor
   *
   * Cleans up all resources, closes remaining windows, and stops event monitoring.
   * This is automatically called when the application terminates.
   */
  virtual ~WindowManager();

  /**
   * @brief Create a new window with the specified options
   *
   * Creates and registers a new window instance with the given configuration.
   * The window is automatically added to the internal window registry and
   * a WindowCreatedEvent is emitted upon successful creation.
   *
   * @param options Configuration options for the new window
   * @return Shared pointer to the created Window instance, or nullptr if creation failed
   *
   * @throws std::runtime_error if window creation fails due to system limitations
   *
   * @code
   * WindowOptions options;
   * options.title = "My Window";
   * options.width = 800;
   * options.height = 600;
   *
   * auto window = WindowManager::GetInstance().Create(options);
   * if (window) {
   *     // Window created successfully
   * }
   * @endcode
   */
  std::shared_ptr<Window> Create(const WindowOptions& options);

  /**
   * @brief Get a window by its unique ID
   *
   * Retrieves a window instance from the internal registry using its ID.
   * This method is useful for accessing windows when you have their ID
   * from events or other sources.
   *
   * @param id The unique identifier of the window to retrieve
   * @return Shared pointer to the Window instance, or nullptr if window not found
   *
   * @code
   * WindowID window_id = some_event.GetWindowId();
   * auto window = WindowManager::GetInstance().Get(window_id);
   * if (window) {
   *     window->Show();
   * }
   * @endcode
   */
  std::shared_ptr<Window> Get(WindowID id);

  /**
   * @brief Get all managed windows
   *
   * Returns a vector containing all currently managed window instances.
   * The returned vector is a snapshot of the current state and modifications
   * to it won't affect the internal window registry.
   *
   * @return Vector of shared pointers to all Window instances
   *
   * @code
   * auto all_windows = WindowManager::GetInstance().GetAll();
   * for (auto& window : all_windows) {
   *     window->Hide();  // Hide all windows
   * }
   * @endcode
   */
  std::vector<std::shared_ptr<Window>> GetAll();

  /**
   * @brief Get the currently active/focused window
   *
   * Returns the window that currently has keyboard focus and is active.
   * This is typically the window that the user is currently interacting with.
   *
   * @return Shared pointer to the current Window instance, or nullptr if no window is active
   *
   * @code
   * auto current = WindowManager::GetInstance().GetCurrent();
   * if (current) {
   *     std::cout << "Active window: " << current->GetTitle() << std::endl;
   * }
   * @endcode
   */
  std::shared_ptr<Window> GetCurrent();

  /**
   * @brief Destroy a window by its ID
   *
   * Removes the specified window from the registry and destroys it.
   * This will close the window, free its resources, and emit a WindowClosedEvent.
   * Any remaining shared_ptr references to the window will become invalid after
   * the window is destroyed.
   *
   * @param id The unique identifier of the window to destroy
   * @return true if the window was found and destroyed, false if window was not found
   *
   * @code
   * WindowID window_id = some_window->GetId();
   * bool success = WindowManager::GetInstance().Destroy(window_id);
   * if (success) {
   *     std::cout << "Window destroyed successfully" << std::endl;
   * }
   * @endcode
   */
  bool Destroy(WindowID id);

 private:
  /**
   * @brief Private constructor to enforce singleton pattern
   *
   * Initializes the WindowManager instance, sets up platform-specific
   * event monitoring, and prepares the internal data structures.
   * This constructor is private to prevent direct instantiation.
   */
  WindowManager();

  // Prevent copy construction and assignment to maintain singleton property
  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;
  WindowManager(WindowManager&&) = delete;
  WindowManager& operator=(WindowManager&&) = delete;

  /**
   * @brief Platform-specific implementation details
   *
   * Uses the PIMPL (Pointer to Implementation) idiom to hide platform-specific
   * details and reduce compilation dependencies.
   */
  class Impl;
  std::unique_ptr<Impl> pimpl_;

  /**
   * @brief Internal registry of all managed windows
   *
   * Maps window IDs to their corresponding Window instances for fast lookup.
   * This container is the authoritative source for all active windows.
   */
  std::unordered_map<WindowID, std::shared_ptr<Window>> windows_;

  /**
   * @brief Set up platform-specific event monitoring
   *
   * Initializes the system for monitoring window events such as creation,
   * destruction, focus changes, etc. This is called during construction.
   */
  void SetupEventMonitoring();

  /**
   * @brief Clean up platform-specific event monitoring
   *
   * Stops event monitoring and cleans up associated resources.
   * This is called during destruction.
   */
  void CleanupEventMonitoring();

  /**
   * @brief Internal method to dispatch window events
   *
   * Processes window events received from the platform and dispatches them
   * to registered event listeners. This method is called by the platform-specific
   * event monitoring system.
   *
   * @param event The window event to dispatch
   */
  void DispatchWindowEvent(const WindowEvent& event);
};

}  // namespace nativeapi