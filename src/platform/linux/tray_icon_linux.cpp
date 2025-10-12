#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>
#include <iostream>
#include <string>
#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl(AppIndicator* indicator)
      : app_indicator_(indicator),
        title_(""),
        tooltip_(""),
        context_menu_(nullptr),
        visible_(false) {}

  AppIndicator* app_indicator_;
  std::shared_ptr<Menu>
      context_menu_;  // Store menu shared_ptr to keep it alive
  std::string title_;
  std::string tooltip_;
  bool visible_;
};

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>(nullptr)) {
  id = -1;

#if HAS_GTK && HAS_AYATANA_APPINDICATOR
  // Create a unique ID for this tray icon
  static int next_indicator_id = 1;
  std::string indicator_id =
      "nativeapi-tray-" + std::to_string(next_indicator_id++);

  // Create a new tray using AppIndicator
  AppIndicator* app_indicator =
      app_indicator_new(indicator_id.c_str(),
                        "application-default-icon",  // Default icon name
                        APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  if (app_indicator) {
    // Reinitialize the Impl with the created indicator
    pimpl_ = std::make_unique<Impl>(app_indicator);
    pimpl_->visible_ = true;
  }
#endif
}

TrayIcon::TrayIcon(void* tray)
    : pimpl_(std::make_unique<Impl>((AppIndicator*)tray)) {
  id = -1;  // Will be set by TrayManager when created
  // Make the indicator visible by default
  if (pimpl_->app_indicator_) {
    pimpl_->visible_ = true;
  }
}

TrayIcon::~TrayIcon() {
  if (pimpl_->app_indicator_) {
    g_object_unref(pimpl_->app_indicator_);
  }
}

void TrayIcon::SetIcon(std::string icon) {
  if (!pimpl_->app_indicator_) {
    return;
  }

  // Check if the icon is a base64 string
  if (icon.find("data:image") != std::string::npos) {
    // For base64 images, we need to save them to a temporary file
    // since AppIndicator expects file paths or stock icon names
    size_t pos = icon.find("base64,");
    if (pos != std::string::npos) {
      std::string base64Icon = icon.substr(pos + 7);

      // Decode base64 data
      gsize decoded_len;
      guchar* decoded_data = g_base64_decode(base64Icon.c_str(), &decoded_len);

      if (decoded_data) {
        // Create a temporary file path
        const char* temp_dir = g_get_tmp_dir();
        std::string temp_path = std::string(temp_dir) +
                                "/nativeapi_tray_icon_" + std::to_string(id) +
                                ".png";

        // Write to file
        GError* error = nullptr;
        if (g_file_set_contents(temp_path.c_str(), (const gchar*)decoded_data,
                                decoded_len, &error)) {
          app_indicator_set_icon_full(pimpl_->app_indicator_, temp_path.c_str(),
                                      "Tray Icon");
        } else if (error) {
          std::cerr << "Error saving icon to temp file: " << error->message
                    << std::endl;
          g_error_free(error);
        }

        g_free(decoded_data);
      }
    }
  } else {
    // Use the icon as a file path or stock icon name
    if (g_file_test(icon.c_str(), G_FILE_TEST_EXISTS)) {
      // It's a file path
      app_indicator_set_icon_full(pimpl_->app_indicator_, icon.c_str(),
                                  "Tray Icon");
    } else {
      // Try as a stock icon name
      app_indicator_set_icon_full(pimpl_->app_indicator_, icon.c_str(),
                                  "Tray Icon");
    }
  }
}

void TrayIcon::SetTitle(std::string title) {
  pimpl_->title_ = title;
  // AppIndicator uses the title as the accessible name and in some desktop
  // environments
  if (pimpl_->app_indicator_) {
    app_indicator_set_title(pimpl_->app_indicator_, title.c_str());
  }
}

std::string TrayIcon::GetTitle() {
  return pimpl_->title_;
}

void TrayIcon::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;
  // AppIndicator doesn't have direct tooltip support like GtkStatusIcon
  // The tooltip functionality is typically handled through the title
  // or through custom menu items. We'll store it for potential future use.
}

std::string TrayIcon::GetTooltip() {
  return pimpl_->tooltip_;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  // Store the menu shared_ptr to keep it alive
  pimpl_->context_menu_ = menu;

  // AppIndicator requires a menu to be set
  if (pimpl_->app_indicator_ && menu && menu->GetNativeObject()) {
    GtkMenu* gtk_menu = static_cast<GtkMenu*>(menu->GetNativeObject());
    app_indicator_set_menu(pimpl_->app_indicator_, gtk_menu);
  }
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  // AppIndicator doesn't provide geometry information like GtkStatusIcon did
  // This is a limitation of the AppIndicator API as it's handled by the
  // system tray implementation. We return empty bounds.
  // In most modern desktop environments, this information isn't available
  // to applications for security reasons.

  return bounds;
}

bool TrayIcon::SetVisible(bool visible) {
  if (!pimpl_->app_indicator_) {
    return false;
  }

  if (visible) {
    app_indicator_set_status(pimpl_->app_indicator_,
                             APP_INDICATOR_STATUS_ACTIVE);
  } else {
    app_indicator_set_status(pimpl_->app_indicator_,
                             APP_INDICATOR_STATUS_PASSIVE);
  }

  pimpl_->visible_ = visible;
  return true;
}

bool TrayIcon::IsVisible() {
  if (pimpl_->app_indicator_) {
    AppIndicatorStatus status =
        app_indicator_get_status(pimpl_->app_indicator_);
    return status == APP_INDICATOR_STATUS_ACTIVE;
  }
  return false;
}

bool TrayIcon::OpenContextMenu(double x, double y) {
  if (!pimpl_->context_menu_ || !pimpl_->context_menu_->GetNativeObject()) {
    return false;
  }

  // AppIndicator shows context menu automatically on right-click
  // We don't need to manually show it at specific coordinates
  // The menu is managed by the indicator framework
  return true;
}

bool TrayIcon::OpenContextMenu() {
  if (!pimpl_->context_menu_ || !pimpl_->context_menu_->GetNativeObject()) {
    return false;
  }

  // AppIndicator shows context menu automatically on right-click
  // We don't need to manually show it as it's managed by the indicator
  // framework
  return true;
}

bool TrayIcon::CloseContextMenu() {
  if (!pimpl_->context_menu_) {
    return true;  // No menu to close, consider success
  }

  // AppIndicator manages menu visibility automatically
  // There's no direct way to programmatically close the menu
  // but we can return true as the operation is conceptually successful
  return true;
}

}  // namespace nativeapi
