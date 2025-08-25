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

MenuItem::MenuItem() : pimpl_(new Impl(nullptr)) {
  id = -1;
}

MenuItem::MenuItem(void* menu_item) : pimpl_(new Impl((GtkWidget*)menu_item)) {
  id = -1;
}

MenuItem::~MenuItem() {
  delete pimpl_;
}

void MenuItem::SetIcon(std::string icon) {
  pimpl_->icon_ = icon;
  // TODO: Implement icon setting for GTK menu item
}

std::string MenuItem::GetIcon() {
  return pimpl_->icon_;
}

void MenuItem::SetTitle(std::string title) {
  pimpl_->title_ = title;
  if (pimpl_->gtk_menu_item_) {
    gtk_menu_item_set_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_), title.c_str());
  }
}

std::string MenuItem::GetTitle() {
  return pimpl_->title_;
}

void MenuItem::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;
  if (pimpl_->gtk_menu_item_) {
    gtk_widget_set_tooltip_text(pimpl_->gtk_menu_item_, tooltip.c_str());
  }
}

std::string MenuItem::GetTooltip() {
  return pimpl_->tooltip_;
}

// Private implementation class for Menu
class Menu::Impl {
 public:
  Impl(GtkWidget* menu) : gtk_menu_(menu) {}
  
  GtkWidget* gtk_menu_;
};

Menu::Menu() : pimpl_(new Impl(gtk_menu_new())) {
  id = -1;
}

Menu::Menu(void* menu) : pimpl_(new Impl((GtkWidget*)menu)) {
  id = -1;
}

Menu::~Menu() {
  if (pimpl_->gtk_menu_) {
    g_object_unref(pimpl_->gtk_menu_);
  }
  delete pimpl_;
}

void Menu::AddItem(MenuItem item) {
  if (pimpl_->gtk_menu_ && item.pimpl_->gtk_menu_item_) {
    gtk_menu_shell_append(GTK_MENU_SHELL(pimpl_->gtk_menu_), item.pimpl_->gtk_menu_item_);
  }
}

void Menu::RemoveItem(MenuItem item) {
  if (pimpl_->gtk_menu_ && item.pimpl_->gtk_menu_item_) {
    gtk_container_remove(GTK_CONTAINER(pimpl_->gtk_menu_), item.pimpl_->gtk_menu_item_);
  }
}

void Menu::AddSeparator() {
  if (pimpl_->gtk_menu_) {
    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(pimpl_->gtk_menu_), separator);
  }
}

MenuItem Menu::CreateItem(std::string title) {
  GtkWidget* menu_item = gtk_menu_item_new_with_label(title.c_str());
  MenuItem item(menu_item);
  item.SetTitle(title);
  return item;
}

MenuItem Menu::CreateItem(std::string title, std::string icon) {
  MenuItem item = CreateItem(title);
  item.SetIcon(icon);
  return item;
}

void* Menu::GetNativeMenu() {
  return (void*)pimpl_->gtk_menu_;
}

}  // namespace nativeapi