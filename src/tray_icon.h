#pragma once

#include <string>
#include "geometry.h"
#include "menu.h"

namespace nativeapi {

typedef long TrayIconID;

class TrayIcon {
 public:
  TrayIcon();
  TrayIcon(void* tray);
  virtual ~TrayIcon();

  TrayIconID id;

  void SetIcon(std::string icon);

  void SetTitle(std::string title);
  std::string GetTitle();

  void SetTooltip(std::string tooltip);
  std::string GetTooltip();

  void SetContextMenu(Menu menu);
  Menu GetContextMenu();

  Rectangle GetBounds();

 private:
  class Impl;
  Impl* pimpl_;
};

}  // namespace nativeapi
