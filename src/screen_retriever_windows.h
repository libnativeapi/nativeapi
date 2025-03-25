#pragma once

#include "screen_retriever.h"

// Windows implementation of ScreenRetriever
class ScreenRetrieverWindows : public ScreenRetriever {
 public:
  ScreenRetrieverWindows();
  ~ScreenRetrieverWindows() override;

  CursorPoint GetCursorScreenPoint() override;
  Display GetPrimaryDisplay() override;
  DisplayList GetAllDisplays() override;
};