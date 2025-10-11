#pragma once

#include <functional>
#include <optional>
#include <string>
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "menu.h"

namespace nativeapi {

typedef long TrayIconID;

/**
 * @brief TrayIcon represents a system tray icon (notification area icon).
 *
 * This class provides a cross-platform interface for creating and managing
 * system tray icons. System tray icons appear in the notification area of
 * the desktop and provide quick access to application functionality through
 * context menus and click events.
 *
 * The class supports:
 * - Setting custom icons (including base64-encoded images)
 * - Displaying text titles and tooltips
 * - Context menus for user interaction
 * - Event emission for mouse clicks (TrayIconClickedEvent,
 * TrayIconRightClickedEvent, TrayIconDoubleClickedEvent)
 * - Visibility control
 *
 * @note This class uses the PIMPL idiom to hide platform-specific
 * implementation details and ensure binary compatibility across different
 * platforms.
 *
 * @example
 * ```cpp
 * // Create a tray icon
 * auto trayIcon = std::make_shared<TrayIcon>();
 * trayIcon->SetIcon("path/to/icon.png");
 * trayIcon->SetTooltip("My Application");
 *
 * // Set up event listeners
 * trayIcon->AddListener<TrayIconClickedEvent>([](const TrayIconClickedEvent&
 * event) {
 *     // Handle left click - show/hide main window
 *     mainWindow->IsVisible() ? mainWindow->Hide() : mainWindow->Show();
 * });
 *
 * trayIcon->AddListener<TrayIconRightClickedEvent>([](const
 * TrayIconRightClickedEvent& event) {
 *     // Handle right click - open context menu
 *     trayIcon->OpenContextMenu();
 * });
 *
 * // Set up a context menu
 * Menu menu;
 * auto item = menu.CreateItem("Exit");
 * menu.AddItem(item);
 * trayIcon->SetContextMenu(menu);
 *
 * // Show the tray icon
 * trayIcon->Show();
 * ```
 */
class TrayIcon : public EventEmitter {
 public:
  /**
   * @brief Default constructor for TrayIcon.
   *
   * Creates a new tray icon instance with default settings.
   * The icon will not be visible until Show() is called.
   */
  TrayIcon();

  /**
   * @brief Constructor that wraps an existing platform-specific tray icon.
   *
   * This constructor is typically used internally by the TrayManager
   * to wrap existing system tray icons.
   *
   * @param tray Pointer to the platform-specific tray icon object
   */
  TrayIcon(void* tray);

  /**
   * @brief Destructor for TrayIcon.
   *
   * Cleans up the tray icon and removes it from the system tray if visible.
   * Also releases any associated platform-specific resources.
   */
  virtual ~TrayIcon();

  /**
   * @brief Unique identifier for this tray icon.
   *
   * This ID is assigned by the TrayManager when the icon is created
   * and can be used to reference the icon in subsequent operations.
   */
  TrayIconID id;

  /**
   * @brief Set the icon image for the tray icon.
   *
   * The icon can be specified as either a file path or a base64-encoded
   * image string. Base64 strings should be prefixed with the data URI
   * scheme (e.g., "data:image/png;base64,iVBORw0KGgo...").
   *
   * @param icon File path to an icon image or base64-encoded image data
   *
   * @note Supported formats depend on the platform:
   *       - macOS: PNG, JPEG, GIF, TIFF, BMP
   *       - Windows: ICO, PNG, BMP
   *       - Linux: PNG, XPM, SVG (depends on desktop environment)
   *
   * @example
   * ```cpp
   * // Using file path
   * trayIcon->SetIcon("/path/to/icon.png");
   *
   * // Using base64 data
   * trayIcon->SetIcon("data:image/png;base64,iVBORw0KGgo...");
   * ```
   */
  void SetIcon(std::string icon);

  /**
   * @brief Set the title text for the tray icon.
   *
   * On platforms that support it (primarily macOS), the title text
   * is displayed next to the icon in the status bar. On other platforms,
   * this may be used internally for identification purposes.
   *
   * @param title The title text to display, or std::nullopt to clear the title
   *
   * @note On Windows and most Linux desktop environments, tray icons
   *       do not display title text directly.
   */
  void SetTitle(std::optional<std::string> title);

  /**
   * @brief Get the current title text of the tray icon.
   *
   * @return The current title text as an optional string, or std::nullopt if no title is set
   */
  std::optional<std::string> GetTitle();

  /**
   * @brief Set the tooltip text for the tray icon.
   *
   * The tooltip appears when the user hovers the mouse over the tray icon.
   * This is supported on all platforms and is useful for providing
   * additional context about the application's current state.
   *
   * @param tooltip The tooltip text to display on hover, or std::nullopt to clear the tooltip
   *
   * @example
   * ```cpp
   * trayIcon->SetTooltip("MyApp - Status: Connected");
   * trayIcon->SetTooltip(std::nullopt); // Clear tooltip
   * ```
   */
  void SetTooltip(std::optional<std::string> tooltip);

  /**
   * @brief Get the current tooltip text of the tray icon.
   *
   * @return The current tooltip text as an optional string, or std::nullopt if no tooltip is set
   */
  std::optional<std::string> GetTooltip();

  /**
   * @brief Set the context menu for the tray icon.
   *
   * The context menu is displayed when the user right-clicks (or equivalent
   * platform-specific action) on the tray icon. The menu provides the primary
   * interface for user interaction with the application.
   *
   * @param menu The Menu object containing the context menu items
   *
   * @note The Menu object is copied internally, so the original menu
   *       object's lifetime doesn't need to extend beyond this call.
   *
   * @example
   * ```cpp
   * Menu contextMenu;
   * contextMenu.AddItem(contextMenu.CreateItem("Show Window"));
   * contextMenu.AddSeparator();
   * contextMenu.AddItem(contextMenu.CreateItem("Exit"));
   * trayIcon->SetContextMenu(contextMenu);
   * ```
   */
  void SetContextMenu(std::shared_ptr<Menu> menu);

  /**
   * @brief Get the current context menu of the tray icon.
   *
   * @return A copy of the current context Menu object
   */
  std::shared_ptr<Menu> GetContextMenu();

  /**
   * @brief Get the screen coordinates and dimensions of the tray icon.
   *
   * Returns the bounding rectangle of the tray icon in screen coordinates.
   * This can be useful for positioning popup windows or dialogs relative
   * to the tray icon.
   *
   * @return Rectangle containing the screen position and size of the tray icon
   *
   * @note The accuracy of this information varies by platform:
   *       - macOS: Precise bounds of the status item
   *       - Windows: Approximate location of the notification area
   *       - Linux: Depends on the desktop environment and system tray
   * implementation
   */
  Rectangle GetBounds();

  /**
   * @brief Set the visibility of the tray icon in the system tray.
   *
   * Controls whether the tray icon is visible in the system notification area.
   * This method replaces the previous Show() and Hide() methods for a more
   * unified interface.
   *
   * @param visible true to make the icon visible, false to hide it
   * @return true if the visibility was successfully changed, false otherwise
   *
   * @note On some platforms, showing a tray icon may fail if the
   *       system tray is not available or if there are too many icons.
   *
   * @example
   * ```cpp
   * // Show the tray icon
   * trayIcon->SetVisible(true);
   *
   * // Hide the tray icon
   * trayIcon->SetVisible(false);
   * ```
   */
  bool SetVisible(bool visible);

  /**
   * @brief Check if the tray icon is currently visible.
   *
   * @return true if the icon is visible in the system tray, false otherwise
   */
  bool IsVisible();

  /**
   * @brief Programmatically display the context menu at a specified location.
   *
   * Opens the tray icon's context menu at the given screen coordinates.
   * This allows for manually triggering the context menu through keyboard
   * shortcuts, other UI events, or programmatic control.
   *
   * @param x The x-coordinate in screen coordinates where to open the menu
   * @param y The y-coordinate in screen coordinates where to open the menu
   * @return true if the menu was successfully opened, false otherwise
   *
   * @note If no context menu has been set via SetContextMenu(), this method
   *       will return false. The coordinates are in screen/global coordinates,
   *       not relative to any window.
   *
   * @example
   * ```cpp
   * // Open context menu at current cursor position
   * POINT cursor;
   * GetCursorPos(&cursor);  // Windows example
   * trayIcon->OpenContextMenu(cursor.x, cursor.y);
   *
   * // Open context menu near the tray icon
   * Rectangle bounds = trayIcon->GetBounds();
   * trayIcon->OpenContextMenu(bounds.x, bounds.y + bounds.height);
   * ```
   */
  bool OpenContextMenu(double x, double y);

  /**
   * @brief Display the context menu at the tray icon's location.
   *
   * Opens the context menu at a default position near the tray icon.
   * This is a convenience method that automatically determines an appropriate
   * position based on the tray icon's current location.
   *
   * @return true if the menu was successfully opened, false otherwise
   *
   * @note The exact positioning behavior may vary by platform:
   *       - macOS: Menu appears below the status item
   *       - Windows: Menu appears near the notification area
   *       - Linux: Menu appears at cursor position or near tray area
   *
   * @example
   * ```cpp
   * // Open context menu at default location
   * trayIcon->OpenContextMenu();
   * ```
   */
  bool OpenContextMenu();

  /**
   * @brief Close the currently displayed context menu.
   *
   * Closes the tray icon's context menu if it is currently visible.
   * This allows for programmatic dismissal of the menu.
   *
   * @return true if the menu was successfully closed or wasn't visible, false
   * on error
   *
   * @note This method is useful for keyboard shortcuts or programmatic control
   *       that needs to dismiss the context menu without user interaction.
   *
   * @example
   * ```cpp
   * // Close the context menu programmatically
   * trayIcon->CloseContextMenu();
   * ```
   */
  bool CloseContextMenu();

  /**
   * @brief Internal method to handle left mouse click events.
   *
   * This method is called internally by platform-specific code
   * when a left click event occurs on the tray icon.
   */
  void HandleLeftClick();

  /**
   * @brief Internal method to handle right mouse click events.
   *
   * This method is called internally by platform-specific code
   * when a right click event occurs on the tray icon.
   */
  void HandleRightClick();

  /**
   * @brief Internal method to handle double click events.
   *
   * This method is called internally by platform-specific code
   * when a double click event occurs on the tray icon.
   */
  void HandleDoubleClick();

  /**
   * @brief Clear the menu reference from the status item.
   *
   * This method is used internally to clean up the menu reference
   * when the context menu is closed. It's called by the menu delegate
   * to ensure proper cleanup.
   */
  void ClearStatusItemMenu();

 private:
  /**
   * @brief Private implementation class using the PIMPL idiom.
   *
   * This forward declaration hides the platform-specific implementation
   * details from the public interface, allowing for better binary
   * compatibility and cleaner separation of concerns.
   */
  class Impl;

  /**
   * @brief Pointer to the private implementation instance.
   *
   * This pointer manages the platform-specific implementation of
   * the tray icon functionality.
   */
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
