#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "foundation/event.h"
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "window.h"

namespace nativeapi {

class Image;

/**
 * @brief What happens to dragged data when it is dropped.
 *
 * A DragSource offers one operation and a DropTarget accepts one; a drop only
 * happens when the two agree.
 */
enum class DragOperation {
  /** Nothing: the drag was cancelled, or the target refused it. */
  None,
  /** The target copies the data; the source keeps it. */
  Copy,
  /** The target takes the data; the source is expected to delete its copy. */
  Move,
  /** The target creates a link (alias, shortcut) to the data. */
  Link
};

/**
 * @brief Base class for events emitted by a DragSource.
 *
 * Every event carries the global cursor position at the moment it was
 * produced, in the same screen coordinate space as Window::GetPosition()
 * (top-left origin, logical pixels).
 */
class DragSourceEvent : public Event {
 public:
  DragSourceEvent(WindowId window_id, Point position)
      : window_id_(window_id), position_(position) {}
  virtual ~DragSourceEvent() = default;

  /**
   * @brief The window the drag was started from.
   */
  WindowId GetWindowId() const { return window_id_; }

  /**
   * @brief The global cursor position in screen coordinates.
   */
  Point GetPosition() const { return position_; }

  std::string GetTypeName() const override { return "DragSourceEvent"; }

 private:
  WindowId window_id_;
  Point position_;
};

/**
 * @brief The drag is over: the data was dropped, or the drag was cancelled.
 */
class DragSourceEndedEvent : public DragSourceEvent {
 public:
  DragSourceEndedEvent(WindowId window_id, Point position, DragOperation operation)
      : DragSourceEvent(window_id, position), operation_(operation) {}

  /**
   * @brief The operation the target performed, or DragOperation::None when
   * nothing was dropped (the user cancelled, or no target accepted the data).
   */
  DragOperation GetOperation() const { return operation_; }

  std::string GetTypeName() const override { return "DragSourceEndedEvent"; }

 private:
  DragOperation operation_;
};

/**
 * @brief Drags files and text out of a window, into other windows and other
 * applications (a file manager, an editor, a mail composer).
 *
 * Describe the data with SetFilePaths() and / or SetText(), then call
 * StartDragging() while the primary mouse button is held down — typically from
 * a mouse-down or drag-start handler. The platform's drag loop takes over the
 * mouse from there; a DragSourceEndedEvent reports how the drag ended.
 *
 * @code
 * auto source = std::make_shared<DragSource>();
 * source->SetFilePaths({"/Users/me/report.pdf"});
 * source->AddListener<DragSourceEndedEvent>([](const DragSourceEndedEvent& event) {
 *   if (event.GetOperation() == DragOperation::None) {
 *     // Nothing was dropped.
 *   }
 * });
 * // In a mouse-down / drag-start handler:
 * source->StartDragging(window);
 * @endcode
 *
 * Events are emitted on the main thread. The data may be changed between
 * drags; changing it during a drag has no effect on that drag.
 *
 * Platform notes:
 * - macOS (AppKit dragging session), Windows (OLE DoDragDrop) and Linux
 *   (GTK 3) are supported.
 * - Android, iOS and OpenHarmony do not support dragging; IsSupported()
 *   returns false and StartDragging() returns false.
 * - The platform drag loop consumes the mouse-up that ends the drag. So that
 *   the window's content does not keep treating the button as held, the window
 *   the drag started from receives a synthesized mouse-up when the drag ends
 *   (macOS, Windows).
 */
class DragSource : public EventEmitter<DragSourceEvent> {
 public:
  /**
   * @brief Checks if dragging data out of a window is available on this
   * platform.
   */
  static bool IsSupported();

  DragSource();
  virtual ~DragSource();

  DragSource(const DragSource&) = delete;
  DragSource& operator=(const DragSource&) = delete;
  DragSource(DragSource&&) = delete;
  DragSource& operator=(DragSource&&) = delete;

  /**
   * @brief Sets the files to drag.
   *
   * @param file_paths Absolute paths of existing files or directories. An
   *        empty list drags no files.
   */
  void SetFilePaths(const std::vector<std::string>& file_paths);

  /**
   * @brief The files the next drag carries.
   */
  std::vector<std::string> GetFilePaths() const;

  /**
   * @brief Sets plain text to drag, alongside or instead of files.
   *
   * @param text UTF-8 text, or std::nullopt to drag no text.
   */
  void SetText(const std::optional<std::string>& text);

  /**
   * @brief The text the next drag carries, if any.
   */
  std::optional<std::string> GetText() const;

  /**
   * @brief Sets the image shown under the cursor while dragging.
   *
   * @param image The drag image, or nullptr for the platform default (the file
   *        icons on macOS; the system drag cursor elsewhere).
   *
   * @note Platform availability:
   * - macOS: ✅ Fully supported - The image replaces the file icons
   * - Windows: ✅ Fully supported - Shown through the shell drag image helper
   * - Linux: ✅ Fully supported - Set as the drag icon
   * - Android: ❌ Not applicable - Always ignored
   * - iOS: ❌ Not applicable - Always ignored
   * - OpenHarmony: ❌ Not applicable - Always ignored
   */
  void SetImage(std::shared_ptr<Image> image);

  /**
   * @brief The drag image, or nullptr for the platform default.
   *
   * @see SetImage() for platform availability.
   */
  std::shared_ptr<Image> GetImage() const;

  /**
   * @brief Sets the operation offered to drop targets.
   *
   * @param operation The operation to offer; defaults to DragOperation::Copy.
   *        DragOperation::Move tells file managers they may move the files
   *        away, so only offer it when that is intended.
   */
  void SetDragOperation(DragOperation operation);

  /**
   * @brief The operation offered to drop targets.
   */
  DragOperation GetDragOperation() const;

  /**
   * @brief Starts dragging the data out of a window.
   *
   * @param window The window the drag starts from. The drag image appears
   *        under the cursor.
   * @return false if the platform does not support dragging, the window is
   *         gone, there is no data to drag (no files and no text), the
   *         operation is DragOperation::None, the primary mouse button is not
   *         held down, or a drag is already in progress.
   *
   * The call returns right away; the drag continues in the platform's drag
   * loop, and DragSourceEndedEvent is emitted when it is over.
   *
   * @note Platform availability:
   * - macOS: ✅ Fully supported - Starts an NSDraggingSession from the content view
   * - Windows: ✅ Fully supported - Runs DoDragDrop on the next turn of the message loop
   * - Linux: ✅ Fully supported - Starts a GTK drag on the window
   * - Android: ❌ Not applicable - Returns false
   * - iOS: ❌ Not applicable - Returns false
   * - OpenHarmony: ❌ Not applicable - Returns false
   */
  bool StartDragging(std::shared_ptr<Window> window);

  /**
   * @brief Checks if a drag started by this source is in progress.
   */
  bool IsDragging() const;

 private:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
