#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "tray_icon.h"

namespace nativeapi {

/**
 * @brief TrayManager is a singleton class that manages all system tray icons.
 *
 * This class provides a centralized way to create, manage, and destroy system
 * tray icons. It ensures that there's only one instance of the tray manager
 * throughout the application lifetime and provides thread-safe operations for
 * managing tray icons.
 *
 * @note This class is implemented as a singleton to ensure consistent
 * management of system tray resources across the entire application.
 */
class TrayManager {
 public:
  /**
   * @brief Get the singleton instance of TrayManager.
   *
   * This method provides access to the unique instance of TrayManager.
   * The instance is created on first call and remains alive for the
   * duration of the application.
   *
   * @return Reference to the singleton TrayManager instance
   * @thread_safety This method is thread-safe
   */
  static TrayManager& GetInstance();

  /**
   * @brief Destructor for TrayManager.
   *
   * Cleans up all managed tray icons and releases system resources.
   */
  virtual ~TrayManager();

  /**
   * @brief Check if the system tray is supported on the current platform.
   *
   * Some platforms or desktop environments may not support system tray
   * functionality. This method allows checking for availability before
   * attempting to create tray icons.
   *
   * @return true if system tray is supported, false otherwise
   */
  bool IsSupported();

  /**
   * @brief Create a new system tray icon.
   *
   * Creates a new tray icon instance and registers it with the manager.
   * The tray icon will be assigned a unique ID for future reference.
   *
   * @return Shared pointer to the newly created TrayIcon instance
   * @note The returned tray icon is automatically managed by this TrayManager
   * @thread_safety This method is thread-safe
   */
  std::shared_ptr<TrayIcon> Create();

  /**
   * @brief Get a tray icon by its unique ID.
   *
   * Retrieves a previously created tray icon using its assigned ID.
   *
   * @param id The unique identifier of the tray icon
   * @return Shared pointer to the TrayIcon if found, nullptr otherwise
   * @thread_safety This method is thread-safe
   */
  std::shared_ptr<TrayIcon> Get(TrayIconID id);

  /**
   * @brief Get all managed tray icons.
   *
   * Returns a vector containing all currently active tray icons
   * managed by this TrayManager instance.
   *
   * @return Vector of shared pointers to all active TrayIcon instances
   * @thread_safety This method is thread-safe
   */
  std::vector<std::shared_ptr<TrayIcon>> GetAll();

  /**
   * @brief Destroy a tray icon by its ID.
   *
   * Removes and destroys a tray icon identified by its unique ID.
   * This will remove the icon from the system tray and clean up
   * associated resources.
   *
   * @param id The unique identifier of the tray icon to destroy
   * @return true if the tray icon was found and destroyed, false otherwise
   * @thread_safety This method is thread-safe
   */
  bool Destroy(TrayIconID id);

  // Prevent copy construction and assignment to maintain singleton property
  TrayManager(const TrayManager&) = delete;
  TrayManager& operator=(const TrayManager&) = delete;
  TrayManager(TrayManager&&) = delete;
  TrayManager& operator=(TrayManager&&) = delete;

 private:
  /**
   * @brief Private constructor to enforce singleton pattern.
   *
   * Initializes the TrayManager instance and sets up initial state.
   */
  TrayManager();

  /**
   * @brief Container for storing active tray icon instances.
   *
   * Maps tray icon IDs to their corresponding TrayIcon instances
   * for efficient lookup and management.
   */
  std::unordered_map<TrayIconID, std::shared_ptr<TrayIcon>> trays_;

  /**
   * @brief ID generator for creating unique tray icon identifiers.
   *
   * Maintains the next available ID to assign to newly created tray icons.
   * This ensures each tray icon has a unique identifier.
   */
  TrayIconID next_tray_id_;

  /**
   * @brief Mutex for thread-safe operations.
   *
   * Protects access to internal data structures to ensure thread safety
   * when multiple threads access the TrayManager simultaneously.
   */
  mutable std::mutex mutex_;
};

}  // namespace nativeapi
