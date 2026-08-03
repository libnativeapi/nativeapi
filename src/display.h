#pragma once
#include <memory>
#include <string>
#include "foundation/event.h"
#include "foundation/geometry.h"
#include "foundation/native_object_provider.h"

namespace nativeapi {

/**
 * Display orientation enumeration
 */
enum class DisplayOrientation {
  kPortrait = 0,
  kLandscape = 90,
  kPortraitFlipped = 180,
  kLandscapeFlipped = 270
};

/**
 * Representation of a display/monitor
 */
class Display : public NativeObjectProvider {
 public:
  Display();
  Display(void* display);
  Display(const Display& other);
  Display& operator=(const Display& other);
  Display(Display&& other) noexcept;
  Display& operator=(Display&& other) noexcept;
  virtual ~Display();

  // Basic identification
  std::string GetId() const;
  std::string GetName() const;

  // Physical properties
  Point GetPosition() const;
  Size GetSize() const;
  Rectangle GetWorkArea() const;
  double GetScaleFactor() const;

  // Additional properties
  bool IsPrimary() const;
  DisplayOrientation GetOrientation() const;
  int GetRefreshRate() const;
  int GetBitDepth() const;

 protected:
  /**
   * @brief Internal method to get the platform-specific native display object.
   *
   * This method must be implemented by platform-specific code to return
   * the underlying native display object.
   *
   * @return Pointer to the native display object
   */
  void* GetNativeObjectInternal() const override;

 private:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

/**
 * Base class for all display-related events
 *
 * This class provides common functionality for display events,
 * including access to the display that triggered the event.
 */
class DisplayEvent : public Event {
 public:
  /**
   * Constructor for DisplayEvent
   * @param display The display associated with this event
   */
  explicit DisplayEvent(const Display& display) : display_(display) {}

  /**
   * Virtual destructor
   */
  virtual ~DisplayEvent() = default;

  /**
   * Get the display associated with this event
   * @return Reference to the display
   */
  const Display& GetDisplay() const { return display_; }

  /**
   * Get a string representation of the event type (for debugging)
   * Default implementation returns "DisplayEvent"
   */
  std::string GetTypeName() const override { return "DisplayEvent"; }

 private:
  Display display_;
};

/**
 * Event class for display addition
 *
 * This event is emitted when a new display is connected to the system.
 */
class DisplayAddedEvent : public DisplayEvent {
 public:
  explicit DisplayAddedEvent(const Display& display) : DisplayEvent(display) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "DisplayAddedEvent"; }
};

/**
 * Event class for display removal
 *
 * This event is emitted when a display is disconnected from the system.
 */
class DisplayRemovedEvent : public DisplayEvent {
 public:
  explicit DisplayRemovedEvent(const Display& display) : DisplayEvent(display) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "DisplayRemovedEvent"; }
};

/**
 * Event class for display configuration changes
 *
 * This event is emitted when a display's properties change (resolution, orientation, etc.).
 */
class DisplayChangedEvent : public DisplayEvent {
 public:
  DisplayChangedEvent(const Display& old_display, const Display& new_display)
      : DisplayEvent(new_display), old_display_(old_display) {}

  /**
   * Get the display information before the change
   * @return Reference to the old display state
   */
  const Display& GetOldDisplay() const { return old_display_; }

  /**
   * Get the display information after the change
   * @return Reference to the new display state
   */
  const Display& GetNewDisplay() const { return GetDisplay(); }

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "DisplayChangedEvent"; }

 private:
  Display old_display_;
};

}  // namespace nativeapi
