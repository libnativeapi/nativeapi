#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <string>
#include "../../display.h"

namespace nativeapi {

// Private implementation class
class Display::Impl {
 public:
  Impl(UIScreen* screen) : ui_screen_(screen) {}
  UIScreen* ui_screen_;
};

Display::Display() : pimpl_(std::make_unique<Impl>([UIScreen mainScreen])) {}

Display::Display(void* display) : pimpl_(std::make_unique<Impl>((__bridge UIScreen*)display)) {}

Display::Display(const Display& other) : pimpl_(std::make_unique<Impl>(other.pimpl_->ui_screen_)) {}

Display& Display::operator=(const Display& other) {
  if (this != &other) {
    pimpl_->ui_screen_ = other.pimpl_->ui_screen_;
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

Display::~Display() {}

std::string Display::GetId() const {
  if (!pimpl_->ui_screen_) {
    return "";
  }

  // Use the screen's bounds as a unique identifier
  CGRect bounds = pimpl_->ui_screen_.bounds;
  return std::to_string((int)bounds.origin.x) + "_" + std::to_string((int)bounds.origin.y);
}

std::string Display::GetName() const {
  if (!pimpl_->ui_screen_) {
    return "Unknown";
  }

  // iOS doesn't provide a friendly name for screens
  // Use scale factor and size to differentiate
  CGFloat scale = pimpl_->ui_screen_.scale;
  CGRect bounds = pimpl_->ui_screen_.bounds;

  return std::string("Screen ") + std::to_string((int)bounds.size.width) + "x" +
         std::to_string((int)bounds.size.height) + "@" + std::to_string((int)scale) + "x";
}

Point Display::GetPosition() const {
  if (!pimpl_->ui_screen_) {
    return Point{0, 0};
  }

  CGRect bounds = pimpl_->ui_screen_.bounds;
  return Point{static_cast<double>(bounds.origin.x), static_cast<double>(bounds.origin.y)};
}

Size Display::GetSize() const {
  if (!pimpl_->ui_screen_) {
    return Size{0, 0};
  }

  CGRect bounds = pimpl_->ui_screen_.bounds;
  return Size{static_cast<double>(bounds.size.width), static_cast<double>(bounds.size.height)};
}

Rectangle Display::GetWorkArea() const {
  if (!pimpl_->ui_screen_) {
    return Rectangle{0, 0, 0, 0};
  }

  CGRect bounds = pimpl_->ui_screen_.bounds;
  return Rectangle{static_cast<double>(bounds.origin.x), static_cast<double>(bounds.origin.y),
                   static_cast<double>(bounds.size.width), static_cast<double>(bounds.size.height)};
}

double Display::GetScaleFactor() const {
  return pimpl_->ui_screen_ ? static_cast<double>(pimpl_->ui_screen_.scale) : 1.0;
}

bool Display::IsPrimary() const {
  return pimpl_->ui_screen_ == [UIScreen mainScreen];
}

DisplayOrientation Display::GetOrientation() const {
  if (!pimpl_->ui_screen_) {
    return DisplayOrientation::kPortrait;
  }

  // Check orientation based on screen bounds
  CGRect bounds = pimpl_->ui_screen_.bounds;
  if (bounds.size.width > bounds.size.height) {
    return DisplayOrientation::kLandscape;
  } else {
    return DisplayOrientation::kPortrait;
  }
}

int Display::GetRefreshRate() const {
  if (!pimpl_->ui_screen_) {
    return 60;
  }

  // iOS doesn't expose refresh rate directly, return standard 60Hz
  return 60;
}

int Display::GetBitDepth() const {
  // iOS devices typically use 32-bit color depth
  return 32;
}

void* Display::GetNativeObjectInternal() const {
  return (__bridge void*)pimpl_->ui_screen_;
}

}  // namespace nativeapi
