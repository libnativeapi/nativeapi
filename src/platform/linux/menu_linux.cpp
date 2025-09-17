#include <iostream>
#include <string>
#include <gtk/gtk.h>
#include "../../menu.h"

namespace nativeapi {

// Private implementation class for MenuItem
class MenuItem::Impl {
 public:
  Impl(GtkWidget* menu_item) : gtk_menu_item_(menu_item), title_(""), icon_(""), tooltip_("") {}
  
  GtkWidget* gtk_menu_item_;
  std::string title_;
  std::string icon_;
  std::string tooltip_;
};

MenuItem::MenuItem(void* menu_item) : pimpl_(new Impl((GtkWidget*)menu_item)) {
}

MenuItem::~MenuItem() {
}

void MenuItem::SetIcon(const std::string& icon) {
  pimpl_->icon_ = icon;
  // TODO: Implement icon setting for GTK menu item
}

std::string MenuItem::GetIcon() const {
  return pimpl_->icon_;
}

void MenuItem::SetTooltip(const std::string& tooltip) {
  pimpl_->tooltip_ = tooltip;
  if (pimpl_->gtk_menu_item_) {
    gtk_widget_set_tooltip_text(pimpl_->gtk_menu_item_, tooltip.c_str());
  }
}

std::string MenuItem::GetTooltip() const {
  return pimpl_->tooltip_;
}

void* MenuItem::GetNativeItem() const {
  return (void*)pimpl_->gtk_menu_item_;
}

// Private implementation class for Menu
class Menu::Impl {
 public:
  Impl(GtkWidget* menu) : gtk_menu_(menu) {}
  
  GtkWidget* gtk_menu_;
};

Menu::Menu(void* menu) : pimpl_(new Impl((GtkWidget*)menu)) {
}

Menu::~Menu() {
  if (pimpl_->gtk_menu_) {
    g_object_unref(pimpl_->gtk_menu_);
  }
}

void Menu::AddItem(std::shared_ptr<MenuItem> item) {
  if (pimpl_->gtk_menu_ && item && item->GetNativeItem()) {
    gtk_menu_shell_append(GTK_MENU_SHELL(pimpl_->gtk_menu_), (GtkWidget*)item->GetNativeItem());
  }
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
  if (pimpl_->gtk_menu_ && item && item->GetNativeItem()) {
    gtk_container_remove(GTK_CONTAINER(pimpl_->gtk_menu_), (GtkWidget*)item->GetNativeItem());
    return true;
  }
  return false;
}

void* Menu::GetNativeMenu() const {
  return (void*)pimpl_->gtk_menu_;
}

}  // namespace nativeapi