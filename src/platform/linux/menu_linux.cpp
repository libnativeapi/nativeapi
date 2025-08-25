#include <iostream>
#include <string>
#include "../../menu.h"

// Include GTK headers
#include <gtk/gtk.h>

namespace nativeapi {

// Private implementation class for MenuItem
class MenuItem::Impl {
 public:
  Impl(GtkWidget* menu_item) : gtk_menu_item_(menu_item) {
    if (gtk_menu_item_) {
      g_object_ref(gtk_menu_item_);
    }
  }
  
  ~Impl() {
    if (gtk_menu_item_) {
      g_object_unref(gtk_menu_item_);
    }
  }
  
  GtkWidget* gtk_menu_item_;
};

MenuItem::MenuItem() : pimpl_(new Impl(gtk_menu_item_new())) {
  id = -1;
}

MenuItem::MenuItem(void* menu_item) : pimpl_(new Impl(static_cast<GtkWidget*>(menu_item))) {
  id = 0;
}

MenuItem::~MenuItem() {
  delete pimpl_;
}

void MenuItem::SetTitle(std::string title) {
  if (pimpl_->gtk_menu_item_) {
    gtk_menu_item_set_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_), title.c_str());
  }
}

std::string MenuItem::GetTitle() {
  if (pimpl_->gtk_menu_item_) {
    const char* label = gtk_menu_item_get_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_));
    return label ? std::string(label) : "";
  }
  return "";
}

void MenuItem::SetIcon(std::string icon) {
  if (!pimpl_->gtk_menu_item_) {
    return;
  }
  
  // For GTK menus, we can use stock icons or create from file
  GtkWidget* image = nullptr;
  
  // Check if the icon is a base64 string
  if (icon.find("data:image") != std::string::npos) {
    // Extract the base64 part
    size_t pos = icon.find("base64,");
    if (pos != std::string::npos) {
      std::string base64Icon = icon.substr(pos + 7);
      
      // For simplicity, we'll use a generic icon for base64 data
      // In a full implementation, you'd decode base64 and create GdkPixbuf
      image = gtk_image_new_from_icon_name("application-x-executable", GTK_ICON_SIZE_MENU);
    }
  } else {
    // Try to load as icon name first, then as file path
    if (g_file_test(icon.c_str(), G_FILE_TEST_EXISTS)) {
      image = gtk_image_new_from_file(icon.c_str());
    } else {
      // Try as icon name
      image = gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_MENU);
    }
  }
  
  if (image) {
    // For GTK3, we need to create an image menu item or set image property
    if (GTK_IS_IMAGE_MENU_ITEM(pimpl_->gtk_menu_item_)) {
      gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pimpl_->gtk_menu_item_), image);
    }
  }
}

std::string MenuItem::GetIcon() {
  if (pimpl_->gtk_menu_item_ && GTK_IS_IMAGE_MENU_ITEM(pimpl_->gtk_menu_item_)) {
    GtkWidget* image = gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(pimpl_->gtk_menu_item_));
    if (image) {
      return "image_set";  // Simplified for now
    }
  }
  return "";
}

void MenuItem::SetTooltip(std::string tooltip) {
  if (pimpl_->gtk_menu_item_) {
    gtk_widget_set_tooltip_text(pimpl_->gtk_menu_item_, tooltip.c_str());
  }
}

std::string MenuItem::GetTooltip() {
  if (pimpl_->gtk_menu_item_) {
    char* tooltip = gtk_widget_get_tooltip_text(pimpl_->gtk_menu_item_);
    if (tooltip) {
      std::string result(tooltip);
      g_free(tooltip);
      return result;
    }
  }
  return "";
}

// Private implementation class for Menu
class Menu::Impl {
 public:
  Impl(GtkWidget* menu) : gtk_menu_(menu) {
    if (gtk_menu_) {
      g_object_ref(gtk_menu_);
    }
  }
  
  ~Impl() {
    if (gtk_menu_) {
      g_object_unref(gtk_menu_);
    }
  }
  
  GtkWidget* gtk_menu_;
};

Menu::Menu() : pimpl_(new Impl(gtk_menu_new())) {
  id = -1;
}

Menu::Menu(void* menu) : pimpl_(new Impl(static_cast<GtkWidget*>(menu))) {
  id = 0;
}

Menu::~Menu() {
  delete pimpl_;
}

void Menu::AddItem(MenuItem item) {
  if (pimpl_->gtk_menu_ && item.pimpl_->gtk_menu_item_) {
    gtk_menu_shell_append(GTK_MENU_SHELL(pimpl_->gtk_menu_), item.pimpl_->gtk_menu_item_);
    gtk_widget_show(item.pimpl_->gtk_menu_item_);
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
    gtk_widget_show(separator);
  }
}

MenuItem Menu::CreateItem(std::string title) {
  GtkWidget* gtk_menu_item = gtk_menu_item_new_with_label(title.c_str());
  return MenuItem(gtk_menu_item);
}

MenuItem Menu::CreateItem(std::string title, std::string icon) {
  // For GTK3, use GtkImageMenuItem for items with icons
  GtkWidget* gtk_menu_item = gtk_image_menu_item_new_with_label(title.c_str());
  MenuItem item(gtk_menu_item);
  item.SetIcon(icon);
  return item;
}

void* Menu::GetNativeMenu() {
  return pimpl_->gtk_menu_;
}

}  // namespace nativeapi