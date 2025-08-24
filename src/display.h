#pragma once
#include <string>
#include "geometry.h"

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
struct Display {
  // Basic identification
  std::string id;    // Unique identifier for the display
  std::string name;  // Human-readable display name

  // Physical properties
  Point position{0.0, 0.0};  // Display position in virtual desktop coordinates
  Size size{0.0, 0.0};       // Full display size in logical pixels
  Rectangle workArea{
      0.0, 0.0, 0.0,
      0.0};  // Available work area (excluding taskbars, docks, etc.)
  double scaleFactor =
      0.0;  // Display scaling factor (1.0 = 100%, 2.0 = 200%, etc.)

  // Additional properties
  bool isPrimary = false;  // Whether this is the primary display
  DisplayOrientation orientation =
      DisplayOrientation::kPortrait;  // Current display orientation
  int refreshRate = 0;                // Refresh rate in Hz (0 if unknown)
  int bitDepth = 0;                   // Color bit depth (0 if unknown)

  // Hardware information
  std::string manufacturer;  // Display manufacturer
  std::string model;         // Display model
  std::string serialNumber;  // Display serial number (if available)
};

}  // namespace nativeapi
