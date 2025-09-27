#pragma once
#include <string>
#include "foundation/native_object_provider.h"
#include "geometry.h"
#include <memory>

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

  // Hardware information
  std::string GetManufacturer() const;
  std::string GetModel() const;
  std::string GetSerialNumber() const;

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

}  // namespace nativeapi
