#include <iostream>
#include <string>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl(GtkStatusIcon* tray) : gtk_status_icon_(tray), title_(""), tooltip_("") {}
  
  GtkStatusIcon* gtk_status_icon_;
  Menu context_menu_;  // Store menu object to keep it alive
  std::string title_;  // GTK StatusIcon doesn't have title, so we store it
  std::string tooltip_;
};

TrayIcon::TrayIcon() : pimpl_(new Impl(nullptr)) {
  id = -1;
}

TrayIcon::TrayIcon(void* tray) : pimpl_(new Impl((GtkStatusIcon*)tray)) {
  id = -1;  // Will be set by TrayManager when created
  // Make the status icon visible
  if (pimpl_->gtk_status_icon_) {
    gtk_status_icon_set_visible(pimpl_->gtk_status_icon_, TRUE);
  }
}

TrayIcon::~TrayIcon() {
  if (pimpl_->gtk_status_icon_) {
    g_object_unref(pimpl_->gtk_status_icon_);
  }
  delete pimpl_;
}

void TrayIcon::SetIcon(std::string icon) {
  if (!pimpl_->gtk_status_icon_) {
    return;
  }

  // Check if the icon is a base64 string
  if (icon.find("data:image") != std::string::npos) {
    // Extract the base64 part
    size_t pos = icon.find("base64,");
    if (pos != std::string::npos) {
      std::string base64Icon = icon.substr(pos + 7);
      
      // Decode base64 data
      gsize decoded_len;
      guchar* decoded_data = g_base64_decode(base64Icon.c_str(), &decoded_len);
      
      if (decoded_data) {
        // Create pixbuf from decoded data
        GInputStream* stream = g_memory_input_stream_new_from_data(
            decoded_data, decoded_len, g_free);
        GError* error = nullptr;
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(stream, nullptr, &error);
        
        if (pixbuf && !error) {
          // Scale pixbuf to appropriate size (24x24 is common for tray icons)
          GdkPixbuf* scaled_pixbuf = gdk_pixbuf_scale_simple(
              pixbuf, 24, 24, GDK_INTERP_BILINEAR);
          
          gtk_status_icon_set_from_pixbuf(pimpl_->gtk_status_icon_, scaled_pixbuf);
          
          g_object_unref(scaled_pixbuf);
          g_object_unref(pixbuf);
        } else if (error) {
          std::cerr << "Error loading icon from base64: " << error->message << std::endl;
          g_error_free(error);
        }
        
        g_object_unref(stream);
      }
    }
  } else {
    // Use the icon as a file path or stock icon name
    if (g_file_test(icon.c_str(), G_FILE_TEST_EXISTS)) {
      // It's a file path
      gtk_status_icon_set_from_file(pimpl_->gtk_status_icon_, icon.c_str());
    } else {
      // Try as a stock icon name
      gtk_status_icon_set_from_icon_name(pimpl_->gtk_status_icon_, icon.c_str());
    }
  }
}

void TrayIcon::SetTitle(std::string title) {
  pimpl_->title_ = title;
  // GTK StatusIcon doesn't support title directly, so we just store it
  // Some desktop environments might show this in tooltips or context
}

std::string TrayIcon::GetTitle() {
  return pimpl_->title_;
}

void TrayIcon::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;
  if (pimpl_->gtk_status_icon_) {
    gtk_status_icon_set_tooltip_text(pimpl_->gtk_status_icon_, tooltip.c_str());
  }
}

std::string TrayIcon::GetTooltip() {
  return pimpl_->tooltip_;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  // Store the menu object to keep it alive
  pimpl_->context_menu_ = *menu;

  // Note: Full GTK integration would need to connect popup-menu signal
  // and show the GTK menu from the Menu object's GetNativeMenu()
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return std::make_shared<Menu>(pimpl_->context_menu_);
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};
  
  if (pimpl_->gtk_status_icon_) {
    GdkScreen* screen;
    GdkRectangle area;
    GtkOrientation orientation;
    
    if (gtk_status_icon_get_geometry(pimpl_->gtk_status_icon_, &screen, &area, &orientation)) {
      bounds.x = area.x;
      bounds.y = area.y;
      bounds.width = area.width;
      bounds.height = area.height;
    }
  }
  
  return bounds;
}

bool TrayIcon::Show() {
  if (pimpl_->gtk_status_icon_) {
    gtk_status_icon_set_visible(pimpl_->gtk_status_icon_, TRUE);
    return true;
  }
  return false;
}

bool TrayIcon::Hide() {
  if (pimpl_->gtk_status_icon_) {
    gtk_status_icon_set_visible(pimpl_->gtk_status_icon_, FALSE);
    return true;
  }
  return false;
}

bool TrayIcon::IsVisible() {
  if (pimpl_->gtk_status_icon_) {
    return gtk_status_icon_get_visible(pimpl_->gtk_status_icon_) == TRUE;
  }
  return false;
}

void TrayIcon::SetOnLeftClick(std::function<void()> callback) {
  // This method is deprecated - use event listeners instead
}

void TrayIcon::SetOnRightClick(std::function<void()> callback) {
  // This method is deprecated - use event listeners instead
}

void TrayIcon::SetOnDoubleClick(std::function<void()> callback) {
  // This method is deprecated - use event listeners instead
}

bool TrayIcon::ShowContextMenu(double x, double y) {
  if (!pimpl_->context_menu_.GetNativeMenu()) {
    return false;
  }

  // Note: GTK implementation would need to show the menu at the specified coordinates
  // This is a simplified implementation
  return false;
}

bool TrayIcon::ShowContextMenu() {
  if (!pimpl_->context_menu_.GetNativeMenu()) {
    return false;
  }

  // Note: GTK implementation would need to show the menu at cursor position
  // This is a simplified implementation
  return false;
}

// Internal method to handle click events
void TrayIcon::HandleLeftClick() {
  try {
    EmitSync<TrayIconClickedEvent>(id, "left");
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::HandleRightClick() {
  try {
    EmitSync<TrayIconRightClickedEvent>(id);
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::HandleDoubleClick() {
  try {
    EmitSync<TrayIconDoubleClickedEvent>(id);
  } catch (...) {
    // Protect against event emission exceptions
  }
}

}  // namespace nativeapi