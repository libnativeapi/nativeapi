#pragma once

#include "screen_retriever.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// macOS implementation of ScreenRetriever
class ScreenRetrieverMacOS : public ScreenRetriever {
 public:
  ScreenRetrieverMacOS();
  ~ScreenRetrieverMacOS() override;

  Point GetCursorScreenPoint() override;
  Display GetPrimaryDisplay() override;
  DisplayList GetAllDisplays() override;

 private:
  // Helper method to create Display struct from NSScreen
  Display CreateDisplayFromNSScreen(NSScreen* screen, bool isMainScreen);
};

}  // namespace nativeapi
