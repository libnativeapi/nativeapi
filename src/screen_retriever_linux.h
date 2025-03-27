#pragma once

#include "screen_retriever.h"

namespace nativeapi {

// Linux implementation of ScreenRetriever
class ScreenRetrieverLinux : public ScreenRetriever {
 public:
  ScreenRetrieverLinux();
  ~ScreenRetrieverLinux() override;

  Point GetCursorScreenPoint() override;
  Display GetPrimaryDisplay() override;
  DisplayList GetAllDisplays() override;
};

}  // namespace nativeapi
