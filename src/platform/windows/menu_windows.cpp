#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "../../menu.h"

namespace nativeapi {

// Global storage for menu item callbacks and data
static std::map<UINT, MenuItem*> g_menu_items;
static UINT g_next_menu_id = 1000;

// Helper function to convert base64 to binary data
std::vector<BYTE> Base64Decode(const std::string& base64) {
  std::vector<BYTE> result;
  // Simple base64 decoder - in production you'd want a more robust implementation
  // For now, return empty vector as placeholder
  return result;
}

// Private implementation class for MenuItem
class MenuItem::Impl {
 public:
  Impl() : menu_id_(g_next_menu_id++), title_(""), tooltip_(""), icon_path_("") {
    g_menu_items[menu_id_] = nullptr; // Will be set by MenuItem constructor
  }
  
  ~Impl() {
    g_menu_items.erase(menu_id_);
  }

  UINT menu_id_;
  std::string title_;
  std::string tooltip_;
  std::string icon_path_;
  HBITMAP icon_bitmap_;
};

MenuItem::MenuItem() : pimpl_(new Impl()) {
  id = pimpl_->menu_id_;
  g_menu_items[pimpl_->menu_id_] = this;
}

MenuItem::MenuItem(void* menu_item) : pimpl_(new Impl()) {
  id = 0; // External menu item
  g_menu_items[pimpl_->menu_id_] = this;
}

MenuItem::~MenuItem() {
  if (pimpl_->icon_bitmap_) {
    DeleteObject(pimpl_->icon_bitmap_);
  }
  delete pimpl_;
}

void MenuItem::SetTitle(std::string title) {
  pimpl_->title_ = title;
}

std::string MenuItem::GetTitle() {
  return pimpl_->title_;
}

void MenuItem::SetIcon(std::string icon) {
  pimpl_->icon_path_ = icon;
  
  // Clean up existing bitmap
  if (pimpl_->icon_bitmap_) {
    DeleteObject(pimpl_->icon_bitmap_);
    pimpl_->icon_bitmap_ = nullptr;
  }

  if (icon.find("data:image") != std::string::npos) {
    // Handle base64 image data
    size_t pos = icon.find("base64,");
    if (pos != std::string::npos) {
      std::string base64Icon = icon.substr(pos + 7);
      // In a full implementation, you would decode the base64 and create an HBITMAP
      // For now, we'll just store the path
    }
  } else if (!icon.empty()) {
    // Load icon from file path
    std::wstring wicon(icon.begin(), icon.end());
    HICON hIcon = (HICON)LoadImageW(nullptr, wicon.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    if (hIcon) {
      // Convert HICON to HBITMAP
      HDC hdc = GetDC(nullptr);
      HDC hdcMem = CreateCompatibleDC(hdc);
      HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 16, 16);
      HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
      
      DrawIconEx(hdcMem, 0, 0, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
      
      SelectObject(hdcMem, hOldBitmap);
      DeleteDC(hdcMem);
      ReleaseDC(nullptr, hdc);
      DestroyIcon(hIcon);
      
      pimpl_->icon_bitmap_ = hBitmap;
    }
  }
}

std::string MenuItem::GetIcon() {
  return pimpl_->icon_path_;
}

void MenuItem::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;
}

std::string MenuItem::GetTooltip() {
  return pimpl_->tooltip_;
}

// Private implementation class for Menu
class Menu::Impl {
 public:
  Impl() : hmenu_(CreatePopupMenu()) {}
  
  Impl(HMENU menu) : hmenu_(menu), owns_menu_(false) {}
  
  ~Impl() {
    if (owns_menu_ && hmenu_) {
      DestroyMenu(hmenu_);
    }
  }

  HMENU hmenu_;
  bool owns_menu_ = true;
  std::vector<MenuItem> items_; // Store items to keep them alive
};

Menu::Menu() : pimpl_(new Impl()) {
  id = reinterpret_cast<MenuID>(pimpl_->hmenu_);
}

Menu::Menu(void* menu) : pimpl_(new Impl(static_cast<HMENU>(menu))) {
  id = 0; // External menu
}

Menu::~Menu() {
  delete pimpl_;
}

void Menu::AddItem(MenuItem item) {
  if (!pimpl_->hmenu_) return;
  
  // Store the item to keep it alive
  pimpl_->items_.push_back(item);
  
  MENUITEMINFOW mii = {};
  mii.cbSize = sizeof(MENUITEMINFOW);
  mii.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
  mii.wID = item.pimpl_->menu_id_;
  
  std::wstring wtitle(item.pimpl_->title_.begin(), item.pimpl_->title_.end());
  mii.dwTypeData = const_cast<LPWSTR>(wtitle.c_str());
  mii.cch = static_cast<UINT>(wtitle.length());
  mii.dwItemData = reinterpret_cast<ULONG_PTR>(&item);

  // Add icon if available
  if (item.pimpl_->icon_bitmap_) {
    mii.fMask |= MIIM_BITMAP;
    mii.hbmpItem = item.pimpl_->icon_bitmap_;
  }

  InsertMenuItemW(pimpl_->hmenu_, GetMenuItemCount(pimpl_->hmenu_), TRUE, &mii);
}

void Menu::RemoveItem(MenuItem item) {
  if (!pimpl_->hmenu_) return;
  
  RemoveMenu(pimpl_->hmenu_, item.pimpl_->menu_id_, MF_BYCOMMAND);
  
  // Remove from our storage
  auto it = std::find_if(pimpl_->items_.begin(), pimpl_->items_.end(),
                         [&item](const MenuItem& stored) {
                           return stored.id == item.id;
                         });
  if (it != pimpl_->items_.end()) {
    pimpl_->items_.erase(it);
  }
}

void Menu::AddSeparator() {
  if (!pimpl_->hmenu_) return;
  
  MENUITEMINFOW mii = {};
  mii.cbSize = sizeof(MENUITEMINFOW);
  mii.fMask = MIIM_FTYPE;
  mii.fType = MFT_SEPARATOR;
  
  InsertMenuItemW(pimpl_->hmenu_, GetMenuItemCount(pimpl_->hmenu_), TRUE, &mii);
}

MenuItem Menu::CreateItem(std::string title) {
  MenuItem item;
  item.SetTitle(title);
  return item;
}

MenuItem Menu::CreateItem(std::string title, std::string icon) {
  MenuItem item;
  item.SetTitle(title);
  item.SetIcon(icon);
  return item;
}

void* Menu::GetNativeMenu() {
  return pimpl_->hmenu_;
}

}  // namespace nativeapi