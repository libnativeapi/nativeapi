#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include "nativeapi.h"

using nativeapi::Display;
using nativeapi::DisplayManager;
using nativeapi::DisplayOrientation;
using nativeapi::Point;

// Helper function to convert orientation enum to string
std::string orientationToString(DisplayOrientation orientation) {
  switch (orientation) {
    case DisplayOrientation::kPortrait:
      return "Portrait (0 deg)";
    case DisplayOrientation::kLandscape:
      return "Landscape (90 deg)";
    case DisplayOrientation::kPortraitFlipped:
      return "Portrait Flipped (180 deg)";
    case DisplayOrientation::kLandscapeFlipped:
      return "Landscape Flipped (270 deg)";
    default:
      return "Unknown";
  }
}

// Helper function to truncate string if it's too long
std::string truncateString(const std::string& str, size_t maxLength) {
  if (str.length() <= maxLength) {
    return str;
  }
  return str.substr(0, maxLength - 3) + "...";
}

// Helper function to format a table row with proper alignment
std::string formatTableRow(const std::string& content, int totalWidth = 70) {
  std::string truncated =
      truncateString(content, totalWidth - 4);  // Leave space for "│ " and " │"
  std::ostringstream oss;
  oss << "│ " << std::left << std::setw(totalWidth - 4) << truncated << " │";
  return oss.str();
}

// Helper function to create table border - using ASCII characters for better
// compatibility
std::string createTableBorder(const std::string& leftChar,
                              const std::string& rightChar,
                              const std::string& fillChar,
                              int width = 70) {
  std::string border;
  if (leftChar == "┌")
    border = "+";
  else if (leftChar == "├")
    border = "+";
  else if (leftChar == "└")
    border = "+";
  else
    border = leftChar;

  border += std::string(width - 2, '-');

  if (rightChar == "┐")
    border += "+";
  else if (rightChar == "┤")
    border += "+";
  else if (rightChar == "┘")
    border += "+";
  else
    border += rightChar;

  return border;
}

// Helper function to print display information in a formatted way
void printDisplayInfo(const Display& display, bool isPrimary = false) {
  const int tableWidth = 70;

  std::cout << createTableBorder("┌", "┐", "─", tableWidth) << std::endl;
  std::cout << formatTableRow("Display: " + display.name, tableWidth)
            << std::endl;
  std::cout << createTableBorder("├", "┤", "─", tableWidth) << std::endl;
  std::cout << formatTableRow("ID: " + display.id, tableWidth) << std::endl;

  // Format position string
  std::string positionStr = "Position: (" +
                            std::to_string((int)display.position.x) + ", " +
                            std::to_string((int)display.position.y) + ")";
  std::cout << formatTableRow(positionStr, tableWidth) << std::endl;

  // Format size string with proper separator
  std::string sizeStr = "Size: " + std::to_string((int)display.size.width) +
                        " x " + std::to_string((int)display.size.height);
  std::cout << formatTableRow(sizeStr, tableWidth) << std::endl;

  // Format work area string with proper formatting
  std::string workAreaStr =
      "Work Area: (" + std::to_string((int)display.workArea.x) + ", " +
      std::to_string((int)display.workArea.y) + ") " +
      std::to_string((int)display.workArea.width) + " x " +
      std::to_string((int)display.workArea.height);
  std::cout << formatTableRow(workAreaStr, tableWidth) << std::endl;

  // Format scale factor string
  std::stringstream scaleStream;
  scaleStream << "Scale Factor: " << std::fixed << std::setprecision(2)
              << display.scaleFactor;
  std::cout << formatTableRow(scaleStream.str(), tableWidth) << std::endl;

  // Format primary status
  std::string primaryStr =
      "Primary: " + std::string(display.isPrimary ? "Yes" : "No");
  std::cout << formatTableRow(primaryStr, tableWidth) << std::endl;

  // Format orientation
  std::string orientationStr =
      "Orientation: " + orientationToString(display.orientation);
  std::cout << formatTableRow(orientationStr, tableWidth) << std::endl;

  // Format refresh rate
  std::string refreshStr =
      "Refresh Rate: " + std::to_string(display.refreshRate) + " Hz";
  std::cout << formatTableRow(refreshStr, tableWidth) << std::endl;

  if (display.bitDepth > 0) {
    std::string bitDepthStr =
        "Bit Depth: " + std::to_string(display.bitDepth) + " bits";
    std::cout << formatTableRow(bitDepthStr, tableWidth) << std::endl;
  }

  if (!display.manufacturer.empty()) {
    std::string manufacturerStr = "Manufacturer: " + display.manufacturer;
    std::cout << formatTableRow(manufacturerStr, tableWidth) << std::endl;
  }

  if (!display.model.empty()) {
    std::string modelStr = "Model: " + display.model;
    std::cout << formatTableRow(modelStr, tableWidth) << std::endl;
  }

  std::cout << createTableBorder("└", "┘", "─", tableWidth) << std::endl;
}

int main() {
  try {
    DisplayManager& displayManager = DisplayManager::GetInstance();

    std::cout << "=== Native API Display Manager Demo ===" << std::endl;
    std::cout << std::endl;

    // Get and display all displays
    std::vector<Display> displays = displayManager.GetAll();

    if (displays.empty()) {
      std::cerr << "❌ No displays found!" << std::endl;
      return 1;
    }

    std::cout << "📺 Found " << displays.size() << " display(s):" << std::endl;
    std::cout << std::endl;

    // Display primary display first
    Display primaryDisplay = displayManager.GetPrimary();
    std::cout << "🌟 PRIMARY DISPLAY:" << std::endl;
    printDisplayInfo(primaryDisplay, true);
    std::cout << std::endl;

    // Display other displays
    if (displays.size() > 1) {
      std::cout << "📱 SECONDARY DISPLAYS:" << std::endl;
      for (const auto& display : displays) {
        if (!display.isPrimary) {
          printDisplayInfo(display);
          std::cout << std::endl;
        }
      }
    }

    // Display cursor position
    Point cursorPos = displayManager.GetCursorPosition();
    std::cout << "🖱️  Current Cursor Position: (" << cursorPos.x << ", "
              << cursorPos.y << ")" << std::endl;
    std::cout << std::endl;

    // Display summary statistics
    double totalWidth = 0, totalHeight = 0;
    double minScaleFactor = displays[0].scaleFactor;
    double maxScaleFactor = displays[0].scaleFactor;

    for (const auto& display : displays) {
      totalWidth += display.size.width;
      totalHeight = std::max(totalHeight, display.size.height);
      minScaleFactor = std::min(minScaleFactor, display.scaleFactor);
      maxScaleFactor = std::max(maxScaleFactor, display.scaleFactor);
    }

    const int summaryWidth = 60;
    std::cout << "📊 SUMMARY:" << std::endl;
    std::cout << createTableBorder("┌", "┐", "─", summaryWidth) << std::endl;

    // Format each summary line properly
    std::string totalDisplaysStr =
        "Total Displays: " + std::to_string(displays.size());
    std::cout << formatTableRow(totalDisplaysStr, summaryWidth) << std::endl;

    std::string combinedWidthStr =
        "Combined Width: " + std::to_string((int)totalWidth);
    std::cout << formatTableRow(combinedWidthStr, summaryWidth) << std::endl;

    std::string maxHeightStr =
        "Max Height: " + std::to_string((int)totalHeight);
    std::cout << formatTableRow(maxHeightStr, summaryWidth) << std::endl;

    std::stringstream scaleRangeStream;
    scaleRangeStream << "Scale Range: " << std::fixed << std::setprecision(1)
                     << minScaleFactor << "x - " << maxScaleFactor << "x";
    std::string scaleRangeStr = scaleRangeStream.str();
    std::cout << formatTableRow(scaleRangeStr, summaryWidth) << std::endl;

    std::cout << createTableBorder("└", "┘", "─", summaryWidth) << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Unknown error occurred" << std::endl;
    return 1;
  }

  std::cout << "\n✅ Display information retrieved successfully!" << std::endl;

  // Test C API
  std::cout << "\n=== Testing C API ===" << std::endl;
  native_display_list_t display_list = native_display_manager_get_all();

  if (display_list.displays != nullptr && display_list.count > 0) {
    std::cout << "C API - Found " << display_list.count
              << " display(s):" << std::endl;
    for (size_t i = 0; i < display_list.count; ++i) {
      const native_display_t& display = display_list.displays[i];
      std::cout << "Display " << (i + 1) << ":" << std::endl;
      std::cout << "  Name: " << (display.name ? display.name : "Unknown") << std::endl;
      std::cout << "  ID: " << (display.id ? display.id : "Unknown") << std::endl;
      std::cout << "  Size: " << display.size.width << " x " << display.size.height << std::endl;
      std::cout << "  Primary: " << (display.is_primary ? "Yes" : "No") << std::endl;
      std::cout << std::endl;
    }

    // Clean up memory
    native_display_list_free(&display_list);
  } else {
    std::cout << "C API - No displays found or error occurred" << std::endl;
  }

  return 0;
}
