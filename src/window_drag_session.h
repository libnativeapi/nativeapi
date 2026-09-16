#pragma once

#include <memory>
#include <string>

#include "foundation/event.h"
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "window.h"

namespace nativeapi {

/**
 * @brief Base class for events emitted by a WindowDragSession.
 *
 * Every event carries the global cursor position at the moment it was produced,
 * in the same screen coordinate space as Window::GetPosition() (top-left origin,
 * logical pixels).
 */
class WindowDragEvent : public Event {
 public:
  WindowDragEvent(WindowId window_id, Point cursor_position)
      : window_id_(window_id), cursor_position_(cursor_position) {}
  virtual ~WindowDragEvent() = default;

  /**
   * @brief The window the session is moving, or 0 for a pointer-only session.
   */
  WindowId GetWindowId() const { return window_id_; }

  /**
   * @brief The global cursor position in screen coordinates.
   */
  Point GetCursorPosition() const { return cursor_position_; }

  std::string GetTypeName() const override { return "WindowDragEvent"; }

 private:
  WindowId window_id_;
  Point cursor_position_;
};

/**
 * @brief The cursor moved while the drag was in progress.
 *
 * When the session drives a window, the window has already been moved to
 * follow the cursor by the time this event is emitted.
 */
class WindowDragMovedEvent : public WindowDragEvent {
 public:
  using WindowDragEvent::WindowDragEvent;
  std::string GetTypeName() const override { return "WindowDragMovedEvent"; }
};

/**
 * @brief The primary mouse button was released; the drag is over.
 */
class WindowDragEndedEvent : public WindowDragEvent {
 public:
  using WindowDragEvent::WindowDragEvent;
  std::string GetTypeName() const override { return "WindowDragEndedEvent"; }
};

/**
 * @brief The drag was stopped by WindowDragSession::Cancel(), or because the
 * window being moved went away.
 */
class WindowDragCancelledEvent : public WindowDragEvent {
 public:
  using WindowDragEvent::WindowDragEvent;
  std::string GetTypeName() const override { return "WindowDragCancelledEvent"; }
};

/**
 * @brief Tracks a pointer drag globally and, optionally, moves a window with it.
 *
 * This is the building block for tear-off tabs and dockable panels: a drag
 * that begins inside one window has to keep going after its content moves to a
 * different window, which the originating window's own mouse events cannot
 * describe. A session follows the primary mouse button across the whole
 * screen, independent of which window receives mouse events, until the button
 * is released.
 *
 * Typical tear-off flow:
 * 1. On mouse-down inside window A, `Start(nullptr, {})` to follow the cursor
 *    without moving anything; watch WindowDragMovedEvent for the cursor
 *    leaving the region the content may be dragged within.
 * 2. When it does, create window B for the content and call
 *    `Start(window_b, anchor)` on the same session. The session hands over
 *    without emitting an end event, and window B now tracks the cursor.
 * 3. While moving, use WindowManager::GetWindowAtPoint() with window B
 *    excluded to find what is under the cursor and highlight drop targets.
 * 4. On WindowDragEndedEvent, either leave window B where it is or move the
 *    content back into the window it was dropped on.
 *
 * Events are emitted synchronously on the main thread. Listeners may call
 * Start() or Cancel() from inside a callback.
 *
 * Platform notes:
 * - macOS, Windows, and Linux (X11) are supported. On Wayland the global cursor
 *   position is not available to applications, so sessions end immediately.
 * - Android, iOS, and OpenHarmony do not support sessions; Start() returns false.
 * - On Windows, positions follow the library-wide convention of physical pixels
 *   divided by the scale factor of the monitor they fall on. The window is
 *   moved in physical pixels internally, so it stays glued to the cursor when
 *   it crosses monitors with different scale factors.
 */
class WindowDragSession : public EventEmitter<WindowDragEvent> {
 public:
  WindowDragSession();
  virtual ~WindowDragSession();

  WindowDragSession(const WindowDragSession&) = delete;
  WindowDragSession& operator=(const WindowDragSession&) = delete;
  WindowDragSession(WindowDragSession&&) = delete;
  WindowDragSession& operator=(WindowDragSession&&) = delete;

  /**
   * @brief Start following the cursor until the primary mouse button is released.
   *
   * @param window The window to move with the cursor, or nullptr to only track
   *        the cursor.
   * @param anchor The point of the window, relative to its top-left frame
   *        corner, that stays under the cursor. Ignored without a window.
   *        Note that the frame includes the title bar; add the offset between
   *        Window::GetBounds() and Window::GetContentBounds() to anchor on a
   *        point inside the content.
   * @return false if the platform cannot track the cursor globally.
   *
   * If the primary button is not held down when the session starts, the window
   * is left where it is and the session ends right away with a
   * WindowDragEndedEvent.
   *
   * Calling Start() on an active session retargets it: the previous target is
   * released without a WindowDragEndedEvent or WindowDragCancelledEvent, so the
   * same gesture can continue with a different window.
   */
  bool Start(std::shared_ptr<Window> window, Point anchor);

  /**
   * @brief Stop the session without waiting for the mouse button to be released.
   *
   * Emits WindowDragCancelledEvent. The window, if any, stays where it is.
   * Does nothing if the session is not active.
   */
  void Cancel();

  /**
   * @brief Whether the session is currently following the cursor.
   */
  bool IsActive() const;

  /**
   * @brief The window being moved, or 0 when idle or tracking the pointer only.
   */
  WindowId GetWindowId() const;

  /**
   * @brief The anchor passed to the most recent Start().
   */
  Point GetAnchor() const;

 private:
  // Shared logic (window_drag_session.cpp).
  void HandleTick();
  void Finish(bool cancelled, Point cursor_position);

  // Platform seam (platform/<os>/window_drag_session_<os>).
  // Starts / stops the source that calls HandleTick() on the main thread while
  // a session is active. Both must be idempotent.
  void StartTicking();
  void StopTicking();
  // Samples the global cursor. Returns false if the platform cannot.
  bool QueryPointer(Point& position, bool& primary_button_down) const;
  // Places `window` so that `anchor_` sits under the cursor at `cursor_position`.
  void MoveWindow(Window& window, Point cursor_position) const;

  class Impl;
  std::unique_ptr<Impl> pimpl_;

  std::shared_ptr<Window> window_;
  WindowId window_id_ = 0;
  Point anchor_{0, 0};
  Point last_cursor_position_{0, 0};
  bool active_ = false;
};

}  // namespace nativeapi
