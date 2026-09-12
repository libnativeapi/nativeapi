#pragma once

#include <windows.h>
#include <memory>
#include "../../menu.h"

namespace nativeapi {

// Private Windows implementation; no WinRT types escape this boundary.
class WinUI3MenuSession {
 public:
  WinUI3MenuSession();
  ~WinUI3MenuSession();
  bool Open(Menu& menu, HWND owner, POINT anchor, Placement placement);
  bool Close();
  static void Refresh(MenuItem& item);

 private:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
