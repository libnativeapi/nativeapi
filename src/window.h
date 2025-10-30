#pragma once
#include <memory>
#include <string>
#include "foundation/geometry.h"
#include "foundation/id_allocator.h"
#include "foundation/native_object_provider.h"

namespace nativeapi {

/**
 * @typedef WindowId
 * @brief Unique identifier for a window instance.
 *
 * This type is used to uniquely identify window instances across the system.
 * Each window gets assigned a unique ID when created.
 */
typedef IdAllocator::IdType WindowId;

/**
 * @struct WindowOptions
 * @brief Configuration options for creating a new window.
 *
 * This struct contains all the parameters needed to configure a window
 * during its creation process. All fields are optional and will use
 * platform defaults if not specified.
 */
struct WindowOptions {
  /** @brief The initial title of the window */
  std::string title;

  /** @brief The initial size of the window */
  Size size;

  /** @brief The minimum size the window can be resized to */
  Size minimum_size;

  /** @brief The maximum size the window can be resized to */
  Size maximum_size;

  /** @brief Whether the window should be centered on the screen when created */
  bool centered;
};

/**
 * @class Window
 * @brief Cross-platform window abstraction class.
 *
 * This class provides a unified interface for creating and managing windows
 * across different operating systems. It encapsulates all window-related
 * functionality including size, position, visibility, focus, and appearance.
 *
 * The Window class uses the PIMPL idiom to hide platform-specific implementation
 * details and provide a clean, consistent API across all supported platforms.
 *
 * @note This class is not thread-safe. All window operations should be performed
 *       on the main UI thread.
 */
class Window : public NativeObjectProvider {
 public:
  /**
   * @brief Default constructor creates a new window with default settings.
   *
   * Creates a new window with platform-default size, position, and properties.
   * The window is initially hidden and must be explicitly shown.
   */
  Window();

  /**
   * @brief Constructor that wraps an existing native window object.
   *
   * @param window Pointer to an existing platform-specific window object
   * @note The Window instance takes ownership of the native window object
   */
  Window(void* window);

  /**
   * @brief Virtual destructor ensures proper cleanup of resources.
   *
   * Destroys the window and releases all associated resources including
   * the native window object.
   */
  virtual ~Window();

  /**
   * @brief Gets the unique identifier for this window.
   *
   * @return WindowId The unique identifier assigned to this window
   */
  WindowId GetId() const;

  // === Focus Management ===

  /**
   * @brief Brings the window to the front and gives it keyboard focus.
   *
   * Makes this window the active window and brings it to the foreground.
   * The window will receive keyboard input after this call.
   */
  void Focus();

  /**
   * @brief Removes keyboard focus from the window.
   *
   * The window will no longer receive keyboard input, but remains visible.
   * Focus may be transferred to another window or removed entirely.
   */
  void Blur();

  /**
   * @brief Checks if the window currently has keyboard focus.
   *
   * @return true if the window has focus, false otherwise
   */
  bool IsFocused() const;

  // === Visibility Management ===

  /**
   * @brief Shows the window and brings it to the front.
   *
   * Makes the window visible and typically gives it focus. If the window
   * was minimized, it will be restored to its previous state.
   */
  void Show();

  /**
   * @brief Shows the window without giving it focus.
   *
   * Makes the window visible but does not change the currently focused window.
   * Useful for showing auxiliary windows or notifications.
   */
  void ShowInactive();

  /**
   * @brief Hides the window from view.
   *
   * Makes the window invisible but does not destroy it. The window can
   * be shown again later with Show() or ShowInactive().
   */
  void Hide();

  /**
   * @brief Checks if the window is currently visible.
   *
   * @return true if the window is visible, false if hidden or minimized
   */
  bool IsVisible() const;
  // === Window State Management ===

  /**
   * @brief Maximizes the window to fill the available screen space.
   *
   * Expands the window to occupy the maximum available area on the screen,
   * typically excluding taskbars and docks.
   */
  void Maximize();

  /**
   * @brief Restores the window from maximized state to its previous size.
   *
   * Returns the window to the size and position it had before being maximized.
   */
  void Unmaximize();

  /**
   * @brief Checks if the window is currently maximized.
   *
   * @return true if the window is maximized, false otherwise
   */
  bool IsMaximized() const;

  /**
   * @brief Minimizes the window, hiding it from the desktop.
   *
   * Reduces the window to an icon in the taskbar or dock. The window
   * remains open but is not visible on the desktop.
   */
  void Minimize();

  /**
   * @brief Restores the window from minimized or maximized state.
   *
   * Returns the window to its normal state and size. If the window was
   * minimized, it becomes visible again. If maximized, it returns to
   * its previous non-maximized size.
   */
  void Restore();

  /**
   * @brief Checks if the window is currently minimized.
   *
   * @return true if the window is minimized, false otherwise
   */
  bool IsMinimized() const;

  /**
   * @brief Sets the window's fullscreen state.
   *
   * @param is_full_screen true to enter fullscreen mode, false to exit
   *
   * In fullscreen mode, the window occupies the entire screen with no
   * window decorations (title bar, borders) visible.
   */
  void SetFullScreen(bool is_full_screen);

  /**
   * @brief Checks if the window is currently in fullscreen mode.
   *
   * @return true if the window is fullscreen, false otherwise
   */
  bool IsFullScreen() const;
  // === Size and Bounds Management ===

  // void SetBackgroundColor(Color color);
  // Color GetBackgroundColor() const;

  /**
   * @brief Sets the window's position and size simultaneously.
   *
   * @param bounds Rectangle containing the desired position and size
   *
   * This method sets both the window's position and size in a single operation,
   * which can be more efficient than separate calls to SetPosition() and SetSize().
   */
  void SetBounds(Rectangle bounds);

  /**
   * @brief Gets the window's current position and size.
   *
   * @return Rectangle containing the current position and size of the window
   *
   * The returned rectangle includes the window frame and decorations.
   */
  Rectangle GetBounds() const;

  /**
   * @brief Sets the position and size of the window's content area.
   *
   * @param bounds Rectangle containing the desired position and size of the content area
   *
   * This method sets both the content area's position and size in a single operation,
   * which can be more efficient than separate calls to SetPosition() and SetContentSize().
   * The content area excludes window decorations like title bar and borders.
   */
  void SetContentBounds(Rectangle bounds);

  /**
   * @brief Gets the position and size of the window's content area.
   *
   * @return Rectangle containing the current position and size of the content area
   *
   * The returned rectangle excludes window decorations and represents the drawable
   * content area of the window.
   */
  Rectangle GetContentBounds() const;

  /**
   * @brief Sets the window's size with optional animation.
   *
   * @param size The new size for the window
   * @param animate Whether to animate the size change
   *
   * Changes the window's outer size including frame and decorations.
   * If animate is true, the resize will be smoothly animated on supported platforms.
   */
  void SetSize(Size size, bool animate);

  /**
   * @brief Gets the window's current outer size.
   *
   * @return Size The current size of the window including frame and decorations
   */
  Size GetSize() const;

  /**
   * @brief Sets the size of the window's content area.
   *
   * @param size The desired size of the content area
   *
   * This sets the size of the drawable content area, excluding window
   * decorations like title bar and borders. The actual window size will
   * be larger to accommodate the frame.
   */
  void SetContentSize(Size size);

  /**
   * @brief Gets the size of the window's content area.
   *
   * @return Size The current size of the content area excluding decorations
   */
  Size GetContentSize() const;

  /**
   * @brief Sets the minimum size the window can be resized to.
   *
   * @param size The minimum allowed size
   *
   * Prevents the user from resizing the window smaller than the specified size.
   * This applies to the outer window size including decorations.
   */
  void SetMinimumSize(Size size);

  /**
   * @brief Gets the current minimum size constraint.
   *
   * @return Size The minimum size the window can be resized to
   */
  Size GetMinimumSize() const;

  /**
   * @brief Sets the maximum size the window can be resized to.
   *
   * @param size The maximum allowed size
   *
   * Prevents the user from resizing the window larger than the specified size.
   * This applies to the outer window size including decorations.
   */
  void SetMaximumSize(Size size);

  /**
   * @brief Gets the current maximum size constraint.
   *
   * @return Size The maximum size the window can be resized to
   */
  Size GetMaximumSize() const;
  // === Window Behavior Properties ===

  /**
   * @brief Sets whether the window can be resized by the user.
   *
   * @param is_resizable true to allow resizing, false to disable
   *
   * When disabled, the user cannot resize the window by dragging its edges
   * or corners. Programmatic resizing via SetSize() is still possible.
   */
  void SetResizable(bool is_resizable);

  /**
   * @brief Checks if the window can be resized by the user.
   *
   * @return true if user can resize the window, false otherwise
   */
  bool IsResizable() const;

  /**
   * @brief Sets whether the window can be moved by the user.
   *
   * @param is_movable true to allow moving, false to disable
   *
   * When disabled, the user cannot move the window by dragging its title bar.
   * Programmatic positioning via SetPosition() is still possible.
   */
  void SetMovable(bool is_movable);

  /**
   * @brief Checks if the window can be moved by the user.
   *
   * @return true if user can move the window, false otherwise
   */
  bool IsMovable() const;

  /**
   * @brief Sets whether the window can be minimized by the user.
   *
   * @param is_minimizable true to allow minimizing, false to disable
   *
   * Controls the availability of minimize functionality in the window's
   * title bar and system menu. Programmatic minimizing is still possible.
   */
  void SetMinimizable(bool is_minimizable);

  /**
   * @brief Checks if the window can be minimized by the user.
   *
   * @return true if user can minimize the window, false otherwise
   */
  bool IsMinimizable() const;

  /**
   * @brief Sets whether the window can be maximized by the user.
   *
   * @param is_maximizable true to allow maximizing, false to disable
   *
   * Controls the availability of maximize functionality in the window's
   * title bar and system menu. Programmatic maximizing is still possible.
   */
  void SetMaximizable(bool is_maximizable);

  /**
   * @brief Checks if the window can be maximized by the user.
   *
   * @return true if user can maximize the window, false otherwise
   */
  bool IsMaximizable() const;

  /**
   * @brief Sets whether the window can enter fullscreen mode.
   *
   * @param is_full_screenable true to allow fullscreen, false to disable
   *
   * Controls whether the window supports fullscreen mode. On some platforms,
   * this affects the availability of fullscreen controls in the UI.
   */
  void SetFullScreenable(bool is_full_screenable);

  /**
   * @brief Checks if the window supports fullscreen mode.
   *
   * @return true if fullscreen is supported, false otherwise
   */
  bool IsFullScreenable() const;

  /**
   * @brief Sets whether the window can be closed by the user.
   *
   * @param is_closable true to allow closing, false to disable
   *
   * When disabled, the close button in the title bar is hidden or disabled.
   * The window can still be closed programmatically.
   */
  void SetClosable(bool is_closable);

  /**
   * @brief Checks if the window can be closed by the user.
   *
   * @return true if user can close the window, false otherwise
   */
  bool IsClosable() const;

  /**
   * @brief Sets whether the window stays on top of other windows.
   *
   * @param is_always_on_top true to keep on top, false for normal behavior
   *
   * When enabled, the window will remain visible above other windows
   * even when it doesn't have focus.
   */
  void SetAlwaysOnTop(bool is_always_on_top);

  /**
   * @brief Checks if the window is set to always stay on top.
   *
   * @return true if window stays on top, false otherwise
   */
  bool IsAlwaysOnTop() const;

  // === Position and Title ===

  /**
   * @brief Sets the window's position on the screen.
   *
   * @param point The new position for the window's top-left corner
   *
   * Coordinates are relative to the screen's origin (typically top-left).
   */
  void SetPosition(Point point);

  /**
   * @brief Gets the window's current position on the screen.
   *
   * @return Point The position of the window's top-left corner
   */
  Point GetPosition() const;

  /**
   * @brief Sets the text displayed in the window's title bar.
   *
   * @param title The new title text for the window
   */
  void SetTitle(std::string title);

  /**
   * @brief Gets the current title text of the window.
   *
   * @return std::string The current title displayed in the title bar
   */
  std::string GetTitle() const;
  // === Appearance and Advanced Behavior ===

  /**
   * @brief Sets whether the window displays a shadow.
   *
   * @param has_shadow true to show shadow, false to hide it
   *
   * Controls the drop shadow effect around the window. On some platforms,
   * this may affect window compositing and visual effects.
   */
  void SetHasShadow(bool has_shadow);

  /**
   * @brief Checks if the window currently displays a shadow.
   *
   * @return true if shadow is enabled, false otherwise
   */
  bool HasShadow() const;

  /**
   * @brief Sets the window's opacity (transparency level).
   *
   * @param opacity Opacity value between 0.0 (fully transparent) and 1.0 (fully opaque)
   *
   * Controls the transparency of the entire window including its content.
   * Values outside the 0.0-1.0 range will be clamped to valid values.
   */
  void SetOpacity(float opacity);

  /**
   * @brief Gets the window's current opacity level.
   *
   * @return float Current opacity value between 0.0 and 1.0
   */
  float GetOpacity() const;

  /**
   * @brief Sets whether the window appears on all virtual desktops/workspaces.
   *
   * @param is_visible_on_all_workspaces true to appear on all workspaces, false for current only
   *
   * When enabled, the window will be visible regardless of which virtual
   * desktop or workspace the user switches to. Platform support may vary.
   */
  void SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces);

  /**
   * @brief Checks if the window appears on all workspaces.
   *
   * @return true if visible on all workspaces, false if only on current workspace
   */
  bool IsVisibleOnAllWorkspaces() const;

  /**
   * @brief Sets whether the window ignores mouse input events.
   *
   * @param is_ignore_mouse_events true to ignore mouse events, false to receive them
   *
   * When enabled, mouse events (clicks, hovers, etc.) pass through the window
   * to whatever is behind it. Useful for overlay or heads-up display windows.
   */
  void SetIgnoreMouseEvents(bool is_ignore_mouse_events);

  /**
   * @brief Checks if the window ignores mouse events.
   *
   * @return true if mouse events are ignored, false if they are received
   */
  bool IsIgnoreMouseEvents() const;

  /**
   * @brief Sets whether the window can receive keyboard focus.
   *
   * @param is_focusable true to allow focus, false to prevent it
   *
   * When disabled, the window cannot receive keyboard focus and will not
   * respond to keyboard input. Useful for utility or informational windows.
   */
  void SetFocusable(bool is_focusable);

  /**
   * @brief Checks if the window can receive keyboard focus.
   *
   * @return true if the window can be focused, false otherwise
   */
  bool IsFocusable() const;

  // === User Interaction ===

  /**
   * @brief Initiates a user drag operation for moving the window.
   *
   * Allows the user to drag the window by clicking and dragging anywhere
   * within the window's content area, not just the title bar. This is
   * commonly used for frameless windows or custom title bars.
   */
  void StartDragging();

  /**
   * @brief Initiates a user resize operation for the window.
   *
   * Allows the user to resize the window by dragging from the current
   * mouse position. The resize behavior depends on the current cursor
   * position relative to the window edges.
   */
  void StartResizing();

 protected:
  /**
   * @brief Internal method to get the platform-specific native window object.
   *
   * This method must be implemented by platform-specific code to return
   * the underlying native window object.
   *
   * @return Pointer to the native window object
   */
  void* GetNativeObjectInternal() const override;

 private:
  /**
   * @brief Forward declaration of platform-specific implementation class.
   *
   * This class uses the PIMPL (Pointer to Implementation) idiom to hide
   * platform-specific details and reduce compilation dependencies.
   */
  class Impl;

  /** @brief Pointer to the platform-specific implementation */
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
