#pragma once

#include "screen_retriever.h"

// Linux implementation of ScreenRetriever
class ScreenRetrieverLinux : public ScreenRetriever {
 public:
  ScreenRetrieverLinux();
  ~ScreenRetrieverLinux() override;

  CursorPoint GetCursorScreenPoint() override;
  Display GetPrimaryDisplay() override;
  DisplayList GetAllDisplays() override;
};