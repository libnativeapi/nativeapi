#include <iostream>
#include <string>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "../../menu.h"
#include "../../tray.h"

namespace nativeapi {

// Private implementation class
class Tray::Impl {
 public:
  Impl(GtkStatusIcon* tray) : gtk_status_icon_(tray), title_(""), tooltip_("") {}
  
  GtkStatusIcon* gtk_status_icon_;
  std::string title_;  // GTK StatusIcon doesn't have title, so we store it
  std::string tooltip_;
};

Tray::Tray() : pimpl_(new Impl(nullptr)) {
  id = -1;
}

Tray::Tray(void* tray) : pimpl_(new Impl((GtkStatusIcon*)tray)) {
  id = -1;  // Will be set by TrayManager when created
  // Make the status icon visible
  if (pimpl_->gtk_status_icon_) {
    gtk_status_icon_set_visible(pimpl_->gtk_status_icon_, TRUE);
  }
}

Tray::~Tray() {
  if (pimpl_->gtk_status_icon_) {
    g_object_unref(pimpl_->gtk_status_icon_);
  }
  delete pimpl_;
}

void Tray::SetIcon(std::string icon) {
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

void Tray::SetTitle(std::string title) {
  pimpl_->title_ = title;
  // GTK StatusIcon doesn't support title directly, so we just store it
  // Some desktop environments might show this in tooltips or context
}

std::string Tray::GetTitle() {
  return pimpl_->title_;
}

void Tray::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;
  if (pimpl_->gtk_status_icon_) {
    gtk_status_icon_set_tooltip_text(pimpl_->gtk_status_icon_, tooltip.c_str());
  }
}

std::string Tray::GetTooltip() {
  return pimpl_->tooltip_;
}

void Tray::SetContextMenu(Menu menu) {
  // For now, just store the menu - full implementation would need 
  // to connect popup-menu signal and show GTK menu
  // TODO: Implement proper menu integration
}

Menu Tray::GetContextMenu() {
  // Return a default/empty menu for now
  // TODO: Return the stored menu once properly implemented
  return Menu();
}

Rectangle Tray::GetBounds() {
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

}  // namespace nativeapi