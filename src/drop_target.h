#pragma once

#include <memory>
#include <string>
#include <vector>

#include "drag_source.h"
#include "foundation/event.h"
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "window.h"

namespace nativeapi {

/**
 * @brief Base class for events emitted by a DropTarget.
 *
 * Every event carries the cursor position relative to the top-left corner of
 * the window's content area, in logical pixels — the coordinate space of the
 * window's own content (a Flutter view, a web view), not the screen.
 */
class DropTargetEvent : public Event {
 public:
  DropTargetEvent(WindowId window_id, Point position)
      : window_id_(window_id), position_(position) {}
  virtual ~DropTargetEvent() = default;

  /**
   * @brief The window the target is attached to.
   */
  WindowId GetWindowId() const { return window_id_; }

  /**
   * @brief The cursor position relative to the window's content area.
   */
  Point GetPosition() const { return position_; }

  std::string GetTypeName() const override { return "DropTargetEvent"; }

 private:
  WindowId window_id_;
  Point position_;
};

/**
 * @brief A drag carrying files or text entered the window, and the target
 * accepted it.
 *
 * Drags the target does not accept (no files or text, or an operation the
 * source does not offer) produce no events at all.
 */
class DropTargetEnteredEvent : public DropTargetEvent {
 public:
  using DropTargetEvent::DropTargetEvent;
  std::string GetTypeName() const override { return "DropTargetEnteredEvent"; }
};

/**
 * @brief The cursor moved while an accepted drag is over the window.
 */
class DropTargetMovedEvent : public DropTargetEvent {
 public:
  using DropTargetEvent::DropTargetEvent;
  std::string GetTypeName() const override { return "DropTargetMovedEvent"; }
};

/**
 * @brief The drag left the window, or was cancelled, without dropping.
 */
class DropTargetExitedEvent : public DropTargetEvent {
 public:
  using DropTargetEvent::DropTargetEvent;
  std::string GetTypeName() const override { return "DropTargetExitedEvent"; }
};

/**
 * @brief The data was dropped on the window.
 *
 * Ends the drag: no DropTargetExitedEvent follows.
 */
class DropTargetDroppedEvent : public DropTargetEvent {
 public:
  DropTargetDroppedEvent(WindowId window_id,
                         Point position,
                         std::vector<std::string> file_paths,
                         std::string text)
      : DropTargetEvent(window_id, position),
        file_paths_(std::move(file_paths)),
        text_(std::move(text)) {}

  /**
   * @brief Absolute paths of the dropped files and directories; empty when the
   * drag carried none.
   */
  std::vector<std::string> GetFilePaths() const { return file_paths_; }

  /**
   * @brief The dropped plain text (UTF-8); empty when the drag carried none.
   */
  std::string GetText() const { return text_; }

  std::string GetTypeName() const override { return "DropTargetDroppedEvent"; }

 private:
  std::vector<std::string> file_paths_;
  std::string text_;
};

/**
 * @brief Makes a window accept files and text dragged onto it — from a file
 * manager, another application, or another window of this one.
 *
 * The target is live while it has listeners: adding the first listener
 * registers the window as a drop destination, removing the last one (or
 * destroying the target) unregisters it. A window without a live target
 * refuses drops.
 *
 * A drag produces DropTargetEnteredEvent, any number of DropTargetMovedEvent,
 * and then either DropTargetExitedEvent or DropTargetDroppedEvent. The dropped
 * data is only delivered with DropTargetDroppedEvent; platforms do not reliably
 * expose it earlier.
 *
 * @code
 * auto target = std::make_shared<DropTarget>(window);
 * target->AddListener<DropTargetDroppedEvent>([](const DropTargetDroppedEvent& event) {
 *   for (const auto& path : event.GetFilePaths()) {
 *     std::cout << "Dropped " << path << std::endl;
 *   }
 * });
 * @endcode
 *
 * Events are emitted on the main thread.
 *
 * Platform notes:
 * - macOS: a transparent view is layered over the content view; it takes no
 *   mouse events.
 * - Windows: the top-level window is registered with OLE (RegisterDragDrop),
 *   which also covers child windows such as a Flutter view. Registration fails
 *   if another component already registered the same window.
 * - Linux: the GTK window is made a drag destination; child widgets that are
 *   drag destinations themselves take precedence.
 * - Android, iOS and OpenHarmony: IsSupported() returns false and no events
 *   are emitted.
 */
class DropTarget : public EventEmitter<DropTargetEvent> {
 public:
  /**
   * @brief Checks if windows can accept drops on this platform.
   */
  static bool IsSupported();

  /**
   * @brief Creates a drop target for a window.
   *
   * @param window The window to accept drops on. The target keeps a reference
   *        to it.
   */
  explicit DropTarget(std::shared_ptr<Window> window);
  virtual ~DropTarget();

  DropTarget(const DropTarget&) = delete;
  DropTarget& operator=(const DropTarget&) = delete;
  DropTarget(DropTarget&&) = delete;
  DropTarget& operator=(DropTarget&&) = delete;

  /**
   * @brief The window the target is attached to, or 0 if it was created
   * without one.
   */
  WindowId GetWindowId() const;

  /**
   * @brief Sets the operation the target performs on a drop.
   *
   * @param operation The operation to accept; defaults to DragOperation::Copy.
   *        A drag is only accepted when its source offers this operation.
   *        DragOperation::None refuses every drag.
   */
  void SetDropOperation(DragOperation operation);

  /**
   * @brief The operation the target performs on a drop.
   */
  DragOperation GetDropOperation() const;

  /**
   * @brief Checks if the window is currently registered as a drop destination.
   *
   * @return false while the target has no listeners, when the platform does not
   *         support drops, or when registration failed.
   */
  bool IsActive() const;

 protected:
  void StartEventListening() override;
  void StopEventListening() override;

 private:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
