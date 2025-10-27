#include <android/log.h>
#include "../../display.h"

#define LOG_TAG "NativeApi"
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace nativeapi {

class Display::Impl {
 public:
  Impl() {}
};

Display::Display() : pimpl_(std::make_unique<Impl>()) {}
Display::Display(void* display) : pimpl_(std::make_unique<Impl>()) {}
Display::~Display() {}

Display::Display(const Display& other) : pimpl_(std::make_unique<Impl>()) {}
Display& Display::operator=(const Display& other) { return *this; }
Display::Display(Display&& other) noexcept : pimpl_(std::move(other.pimpl_)) {}
Display& Display::operator=(Display&& other) noexcept {
  pimpl_ = std::move(other.pimpl_);
  return *this;
}

void* Display::GetNativeObjectInternal() const {
  return nullptr;
}

std::string Display::GetId() const {
  return "android_display_0";
}

std::string Display::GetName() const {
  return "Android Display";
}

Point Display::GetPosition() const {
  return Point{0, 0};
}

Size Display::GetSize() const {
  return Size{1080, 1920};
}

Rectangle Display::GetWorkArea() const {
  return Rectangle{0, 0, 1080, 1920};
}

double Display::GetScaleFactor() const {
  return 2.0;
}

bool Display::IsPrimary() const {
  return true;
}

DisplayOrientation Display::GetOrientation() const {
  return DisplayOrientation::kPortrait;
}

int Display::GetRefreshRate() const {
  return 60;
}

int Display::GetBitDepth() const {
  return 24;
}

}  // namespace nativeapi
