#pragma once
#include <string>
#include "geometry.h"

namespace nativeapi {

typedef long MenuItemID;

class MenuItem {
 public:
  MenuItem();
  MenuItem(void* menu_item);
  virtual ~MenuItem();

  MenuItemID id;

  void SetIcon(std::string icon);
  std::string GetIcon();

  void SetTitle(std::string title);
  std::string GetTitle();

  void SetTooltip(std::string tooltip);
  std::string GetTooltip();

 private:
  class Impl;
  Impl* pimpl_;
};

typedef long MenuID;

class Menu {
 public:
  Menu();
  Menu(void* menu);
  virtual ~Menu();

  MenuID id;

  void AddItem(MenuItem item);
  void RemoveItem(MenuItem item);

 private:
  class Impl;
  Impl* pimpl_;
};

}  // namespace nativeapi