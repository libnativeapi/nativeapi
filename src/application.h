#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "application_event.h"
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "menu.h"
#include "window.h"

namespace nativeapi {


/**
 * @brief Application is a singleton class that manages the application lifecycle
 *
 * The Application class provides centralized management of application-wide state,
 * lifecycle events, and coordination between different managers. It follows the
 * singleton pattern to ensure there's only one application instance throughout
 * the application lifetime.
 *
 * Key features:
 * - Singleton pattern ensures single application instance
 * - Event-driven architecture for application lifecycle notifications
 * - Cross-platform application management
 * - Integration with existing managers (WindowManager, DisplayManager, etc.)
 * - Thread-safe access to the singleton instance
 * - Automatic cleanup of resources on destruction
 *
 * @note This class is thread-safe for singleton access, but individual operations
 *       may require additional synchronization depending on the platform implementation.
 */
class Application : public EventEmitter<ApplicationEvent> {
 public:
  /**
   * @brief Get the singleton instance of Application
   *
   * This method provides access to the unique instance of Application using
   * the Meyer's singleton pattern. The instance is created on first call and
   * remains alive for the duration of the application. This method is thread-safe
   * and guarantees that only one instance will be created even in multi-threaded
   * environments.
   *
   * @return Reference to the singleton Application instance
   * @thread_safety This method is thread-safe
   *
   * @code
   * // Usage example:
   * auto& app = Application::GetInstance();
   * app.Initialize(options);
   * @endcode
   */
  static Application& GetInstance();

  /**
   * @brief Destructor
   *
   * Cleans up all resources, stops event monitoring, and performs final cleanup.
   * This is automatically called when the application terminates.
   */
  virtual ~Application();

  /**
   * @brief Initialize the application
   *
   * Sets up the application with default configuration. This method
   * should be called before any other application operations. It sets up
   * platform-specific initialization and emits an ApplicationStartedEvent.
   *
   * @return true if initialization succeeded, false otherwise
   *
   * @throws std::runtime_error if initialization fails due to system limitations
   *
   * @code
   * auto& app = Application::GetInstance();
   * if (app.Initialize()) {
   *     // Application initialized successfully
   * }
   * @endcode
   */
  bool Initialize();

  /**
   * @brief Run the application main event loop
   *
   * Starts the main event loop and blocks until the application exits.
   * This method handles platform-specific event processing and coordination
   * between different managers.
   *
   * @return Exit code of the application (0 for success)
   *
   * @code
   * auto& app = Application::GetInstance();
   * int exit_code = app.Run();
   * @endcode
   */
  int Run();

  /**
   * @brief Request the application to quit
   *
   * Initiates the application shutdown process. This method emits an
   * ApplicationQuitRequestedEvent and begins the cleanup process.
   *
   * @param exit_code The exit code to use when quitting (default: 0)
   *
   * @code
   * auto& app = Application::GetInstance();
   * app.Quit(0);  // Quit with success code
   * @endcode
   */
  void Quit(int exit_code = 0);

  /**
   * @brief Check if the application is currently running
   *
   * @return true if the application is running, false otherwise
   */
  bool IsRunning() const;


  /**
   * @brief Check if this is a single instance application
   *
   * @return true if only one instance is allowed, false otherwise
   */
  bool IsSingleInstance() const;

  /**
   * @brief Set the application icon
   *
   * Sets the application icon that appears in the dock (macOS), taskbar (Windows),
   * or application list (Linux).
   *
   * @param icon_path Path to the icon file
   * @return true if the icon was set successfully, false otherwise
   */
  bool SetIcon(const std::string& icon_path);

  /**
   * @brief Show or hide the dock icon (macOS only)
   *
   * Controls whether the application appears in the macOS dock.
   * This method has no effect on other platforms.
   *
   * @param visible true to show the dock icon, false to hide it
   * @return true if the operation succeeded, false otherwise
   */
  bool SetDockIconVisible(bool visible);

  /**
   * @brief Set the application menu bar
   *
   * Sets the application-wide menu bar that appears at the top of the screen.
   * This is primarily used on macOS, but may have effects on other platforms.
   *
   * @param menu Shared pointer to the menu to set as the application menu
   * @return true if the menu was set successfully, false otherwise
   */
  bool SetMenuBar(std::shared_ptr<Menu> menu);

  /**
   * @brief Get the primary window of the application
   *
   * Returns the main window of the application, if one exists.
   *
   * @return Shared pointer to the primary window, or nullptr if none exists
   */
  std::shared_ptr<Window> GetPrimaryWindow() const;

  /**
   * @brief Set the primary window of the application
   *
   * Sets the main window of the application. This window will be used for
   * application-level operations and may receive special treatment from
   * the platform.
   *
   * @param window Shared pointer to the window to set as primary
   */
  void SetPrimaryWindow(std::shared_ptr<Window> window);

  /**
   * @brief Get all application windows
   *
   * Returns a vector containing all windows belonging to this application.
   *
   * @return Vector of shared pointers to all application windows
   */
  std::vector<std::shared_ptr<Window>> GetAllWindows() const;


 private:
  /**
   * @brief Private constructor to enforce singleton pattern
   *
   * Initializes the Application instance and sets up platform-specific
   * event monitoring. This constructor is private to prevent direct instantiation.
   */
  Application();

  // Prevent copy construction and assignment to maintain singleton property
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  /**
   * @brief Platform-specific implementation details
   *
   * Uses the PIMPL (Pointer to Implementation) idiom to hide platform-specific
   * details and reduce compilation dependencies.
   */
  class Impl;
  std::unique_ptr<Impl> pimpl_;


  /**
   * @brief Application state
   */
  bool initialized_;
  bool running_;
  int exit_code_;

  /**
   * @brief Primary application window
   */
  std::shared_ptr<Window> primary_window_;

  /**
   * @brief Clean up platform-specific event monitoring
   *
   * Stops event monitoring and cleans up associated resources.
   * This is called during destruction.
   */
  void CleanupEventMonitoring();

 private:
};

}  // namespace nativeapi
