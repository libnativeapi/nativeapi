#include "screen_retriever_linux.h"
#include <iostream>

namespace nativeapi {

ScreenRetrieverLinux::ScreenRetrieverLinux() {
  // Constructor implementation
  std::cout << "ScreenRetrieverLinux initialized" << std::endl;
}

ScreenRetrieverLinux::~ScreenRetrieverLinux() {
  // Destructor implementation
  std::cout << "ScreenRetrieverLinux destroyed" << std::endl;
}

Point ScreenRetrieverLinux::GetCursorScreenPoint() {
  // Empty implementation
  Point point;
  point.x = 0.0;
  point.y = 0.0;
  return point;
}

Display ScreenRetrieverLinux::GetPrimaryDisplay() {
  // Empty implementation
  Display display;
  display.id = "display-1";
  display.name = "Linux Display";
  display.width = 1920.0;
  display.height = 1080.0;
  display.visiblePositionX = 0.0;
  display.visiblePositionY = 0.0;
  display.visibleSizeWidth = 1920.0;
  display.visibleSizeHeight = 1080.0;
  display.scaleFactor = 1.0;
  return display;
}

DisplayList ScreenRetrieverLinux::GetAllDisplays() {
  // Empty implementation
  DisplayList displayList;
  displayList.displays = new Display[1];
  displayList.displays[0] = GetPrimaryDisplay();
  displayList.count = 1;
  return displayList;
}

}  // namespace nativeapi
