#include <iostream>
#include "libnativeapi.h"

int main() {
  auto screenRetriever = nativeapi::ScreenRetriever::Create();

  // Get primary display information
  nativeapi::Display primaryDisplay = screenRetriever->GetPrimaryDisplay();
  std::cout << "Primary Display Information:" << std::endl;
  std::cout << "ID: " << primaryDisplay.id << std::endl;
  std::cout << "Name: " << primaryDisplay.name << std::endl;
  std::cout << "Resolution: " << primaryDisplay.width << "x"
            << primaryDisplay.height << std::endl;
  std::cout << "Scale Factor: " << primaryDisplay.scaleFactor << std::endl;
  std::cout << "Visible Position: (" << primaryDisplay.visiblePositionX << ", "
            << primaryDisplay.visiblePositionY << ")" << std::endl;
  std::cout << "Visible Size: " << primaryDisplay.visibleSizeWidth << "x"
            << primaryDisplay.visibleSizeHeight << std::endl;
  std::cout << std::endl;

  // Get all displays information
  nativeapi::DisplayList allDisplays = screenRetriever->GetAllDisplays();
  std::cout << "All Displays Information:" << std::endl;
  for (int i = 0; i < allDisplays.count; i++) {
    nativeapi::Display& display = allDisplays.displays[i];
    std::cout << "Display " << (i + 1) << ":" << std::endl;
    std::cout << "ID: " << display.id << std::endl;
    std::cout << "Name: " << display.name << std::endl;
    std::cout << "Resolution: " << display.width << "x" << display.height
              << std::endl;
    std::cout << "Scale Factor: " << display.scaleFactor << std::endl;
    std::cout << "Visible Position: (" << display.visiblePositionX << ", "
              << display.visiblePositionY << ")" << std::endl;
    std::cout << "Visible Size: " << display.visibleSizeWidth << "x"
              << display.visibleSizeHeight << std::endl;
    std::cout << std::endl;
    }

  // Get cursor position
  nativeapi::Point cursorPoint = screenRetriever->GetCursorScreenPoint();
  std::cout << "Current Cursor Position: (" << cursorPoint.x << ", "
            << cursorPoint.y << ")" << std::endl;

  // Clean up memory
  delete[] allDisplays.displays;
  delete[] primaryDisplay.id;
  delete[] primaryDisplay.name;

  return 0;
}