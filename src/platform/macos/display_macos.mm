#include "../../display.h"
#include "coordinate_utils_macos.h"

// Import Cocoa and Core Graphics headers
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

namespace nativeapi {

// Private implementation class
class Display::Impl {
 public:
  Impl() = default;
  Impl(NSScreen* screen) : ns_screen_(screen) {
    if (screen) {
      display_id_ = [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
    }
  }
  Impl(CGDirectDisplayID display_id) : display_id_(display_id) {
    // Find corresponding NSScreen for this display ID
    NSArray<NSScreen*>* screens = [NSScreen screens];
    for (NSScreen* screen in screens) {
      CGDirectDisplayID screenDisplayID =
          [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
      if (screenDisplayID == display_id) {
        ns_screen_ = screen;
        break;
      }
    }
  }

  NSScreen* ns_screen_ = nil;
  CGDirectDisplayID display_id_ = 0;
};

Display::Display() : pimpl_(std::make_unique<Impl>()) {}

Display::Display(void* display) : pimpl_(std::make_unique<Impl>()) {
  if (display) {
    // Assume the void* is either NSScreen* or CGDirectDisplayID*
    // Try NSScreen first
    NSScreen* screen = (__bridge NSScreen*)display;
    if (screen && [screen isKindOfClass:[NSScreen class]]) {
      pimpl_->ns_screen_ = screen;
      pimpl_->display_id_ =
          [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
    } else {
      // Try CGDirectDisplayID
      CGDirectDisplayID displayID = *(CGDirectDisplayID*)display;
      pimpl_->display_id_ = displayID;
      // Find corresponding NSScreen
      NSArray<NSScreen*>* screens = [NSScreen screens];
      for (NSScreen* s in screens) {
        CGDirectDisplayID screenDisplayID =
            [[[s deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
        if (screenDisplayID == displayID) {
          pimpl_->ns_screen_ = s;
          break;
        }
      }
    }
  }
}

Display::Display(const Display& other) : pimpl_(std::make_unique<Impl>(*other.pimpl_)) {}

Display& Display::operator=(const Display& other) {
  if (this != &other) {
    *pimpl_ = *other.pimpl_;
  }
  return *this;
}

Display::Display(Display&& other) noexcept : pimpl_(std::move(other.pimpl_)) {}

Display& Display::operator=(Display&& other) noexcept {
  if (this != &other) {
    pimpl_ = std::move(other.pimpl_);
  }
  return *this;
}

Display::~Display() = default;

void* Display::GetNativeObjectInternal() const {
  return (__bridge void*)pimpl_->ns_screen_;
}

// Getters - directly read from NSScreen
std::string Display::GetId() const {
  if (!pimpl_->ns_screen_)
    return "";
  NSString* screenId = [NSString stringWithFormat:@"%@", @(pimpl_->display_id_)];
  return [screenId UTF8String];
}

std::string Display::GetName() const {
  if (!pimpl_->ns_screen_)
    return "";
  NSString* displayName;
  if (@available(macOS 10.15, *)) {
    displayName = [pimpl_->ns_screen_ localizedName];
  } else {
    displayName = [NSString stringWithFormat:@"Display %@", @(pimpl_->display_id_)];
  }
  return [displayName UTF8String];
}

Point Display::GetPosition() const {
  if (!pimpl_->ns_screen_)
    return {0.0, 0.0};
  NSRect frame = [pimpl_->ns_screen_ frame];

  // Convert from bottom-left (macOS default) to top-left coordinate system
  CGPoint topLeft = NSRectExt::topLeft(frame);

  return {topLeft.x, topLeft.y};
}

Size Display::GetSize() const {
  if (!pimpl_->ns_screen_)
    return {0.0, 0.0};
  NSRect frame = [pimpl_->ns_screen_ frame];
  return {frame.size.width, frame.size.height};
}

Rectangle Display::GetWorkArea() const {
  if (!pimpl_->ns_screen_)
    return {0.0, 0.0, 0.0, 0.0};
  NSRect visibleFrame = [pimpl_->ns_screen_ visibleFrame];

  // Convert from bottom-left (macOS default) to top-left coordinate system
  CGPoint topLeft = NSRectExt::topLeft(visibleFrame);

  return {topLeft.x, topLeft.y, visibleFrame.size.width, visibleFrame.size.height};
}

double Display::GetScaleFactor() const {
  if (!pimpl_->ns_screen_)
    return 1.0;
  return [pimpl_->ns_screen_ backingScaleFactor];
}

bool Display::IsPrimary() const {
  if (!pimpl_->ns_screen_)
    return false;
  NSArray<NSScreen*>* screens = [NSScreen screens];
  return screens.count > 0 && screens[0] == pimpl_->ns_screen_;
}

DisplayOrientation Display::GetOrientation() const {
  if (!pimpl_->ns_screen_)
    return DisplayOrientation::kPortrait;
  NSRect frame = [pimpl_->ns_screen_ frame];
  return (frame.size.width > frame.size.height) ? DisplayOrientation::kLandscape
                                                : DisplayOrientation::kPortrait;
}

int Display::GetRefreshRate() const {
  if (!pimpl_->ns_screen_)
    return 60;
  CGDisplayModeRef displayMode = CGDisplayCopyDisplayMode(pimpl_->display_id_);
  if (displayMode) {
    double refreshRate = CGDisplayModeGetRefreshRate(displayMode);
    CGDisplayModeRelease(displayMode);
    return refreshRate > 0 ? (int)refreshRate : 60;
  }
  return 60;
}

int Display::GetBitDepth() const {
  return 32;  // Default for modern displays
}

}  // namespace nativeapi
