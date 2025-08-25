#include <cstring>
#include <iostream>
#include <string>

#include "../../window.h"
#include "../../window_manager.h"

// Import GTK headers
#include <gtk/gtk.h>
#include <gdk/gdk.h>

namespace nativeapi {

WindowManager::WindowManager() {
  // Initialize GTK if not already initialized (check if default display exists)
  if (!gdk_display_get_default()) {
    gtk_init(nullptr, nullptr);
  }
}

WindowManager::~WindowManager() {}

std::shared_ptr<Window> WindowManager::Get(WindowID id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    return it->second;
  }

  // Try to find the window by ID in the current display
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return nullptr;
  }

  // Get all toplevel windows
  GList* toplevels = gtk_window_list_toplevels();
  for (GList* l = toplevels; l != nullptr; l = l->next) {
    GtkWindow* gtk_window = GTK_WINDOW(l->data);
    GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));
    
    if (gdk_window && (WindowID)gdk_window == id) {
      auto window = std::make_shared<Window>((void*)gdk_window);
      windows_[id] = window;
      g_list_free(toplevels);
      return window;
    }
  }
  g_list_free(toplevels);
  return nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  std::vector<std::shared_ptr<Window>> windows;
  
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return windows;
  }

  // Get all toplevel windows
  GList* toplevels = gtk_window_list_toplevels();
  for (GList* l = toplevels; l != nullptr; l = l->next) {
    GtkWindow* gtk_window = GTK_WINDOW(l->data);
    GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));
    
    if (gdk_window) {
      WindowID window_id = (WindowID)gdk_window;
      auto it = windows_.find(window_id);
      if (it == windows_.end()) {
        auto window = std::make_shared<Window>((void*)gdk_window);
        windows_[window_id] = window;
      }
    }
  }
  g_list_free(toplevels);

  // Return all cached windows
  for (auto& window : windows_) {
    windows.push_back(window.second);
  }
  return windows;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return nullptr;
  }

  // Try to get the focused window
  GdkSeat* seat = gdk_display_get_default_seat(display);
  if (seat) {
    GdkDevice* keyboard = gdk_seat_get_keyboard(seat);
    if (keyboard) {
      GdkWindow* focused_window = gdk_device_get_window_at_position(keyboard, nullptr, nullptr);
      if (focused_window) {
        WindowID window_id = (WindowID)focused_window;
        return Get(window_id);
      }
    }
  }

  // Fallback: get the first visible window
  GList* toplevels = gtk_window_list_toplevels();
  for (GList* l = toplevels; l != nullptr; l = l->next) {
    GtkWindow* gtk_window = GTK_WINDOW(l->data);
    if (gtk_widget_get_visible(GTK_WIDGET(gtk_window))) {
      GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));
      if (gdk_window) {
        WindowID window_id = (WindowID)gdk_window;
        g_list_free(toplevels);
        return Get(window_id);
      }
    }
  }
  g_list_free(toplevels);
  
  return nullptr;
}

}  // namespace nativeapi