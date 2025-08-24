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
  friend class Menu;
  friend class Tray;
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
  void AddSeparator();

  // Convenience methods
  MenuItem CreateItem(std::string title);
  MenuItem CreateItem(std::string title, std::string icon);

  void* GetNativeMenu();

 private:
  friend class Tray;
  class Impl;
  Impl* pimpl_;
};

}  // namespace nativeapi