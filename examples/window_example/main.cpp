#include <iostream>
#include "nativeapi.h"

using nativeapi::AppRunner;
using nativeapi::Display;
using nativeapi::DisplayAddedEvent;
using nativeapi::DisplayManager;
using nativeapi::DisplayRemovedEvent;
using nativeapi::Menu;
using nativeapi::MenuItem;
using nativeapi::MenuItemSelectedEvent;
using nativeapi::MenuItemType;
using nativeapi::TrayIcon;
using nativeapi::TrayManager;
using nativeapi::Window;
using nativeapi::WindowManager;
using nativeapi::WindowOptions;

int main() {
  native_accessibility_manager_enable();
  bool is_enabled = native_accessibility_manager_is_enabled();
  std::cout << "is_enabled: " << is_enabled << std::endl;

  DisplayManager& display_manager = DisplayManager::GetInstance();
  TrayManager& tray_manager = TrayManager::GetInstance();
  WindowManager& window_manager = WindowManager::GetInstance();

  // Create a new window with options
  WindowOptions options = {.title = "Window Example",
                           .size = {800, 600},
                           .minimum_size = {400, 300},
                           .maximum_size = {1920, 1080},
                           .centered = true};
  std::shared_ptr<Window> window_ptr = window_manager.Create(options);

  std::shared_ptr<TrayIcon> tray_icon_ptr = tray_manager.Create();
  if (tray_icon_ptr != nullptr) {
    TrayIcon& tray_icon = *tray_icon_ptr;
    tray_icon.SetIcon(
        "data:image/"
        "png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAABGdBTUEAALGPC/"
        "xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAhGVY"
        "SWZNTQAqAAAACAAFARIAAwAAAAEAAQAAARoABQAAAAEAAABKARsABQAAAAEAAABSASgAAw"
        "AAAAEAAgAAh2kABAAAAAEAAABaAAAAAAAAAEgAAAABAAAASAAAAAEAA6ABAAMAAAABAAEA"
        "AKACAAQAAAABAAAAFKADAAQAAAABAAAAFAAAAABB553+"
        "AAAACXBIWXMAAAsTAAALEwEAmpwYAAABWWlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPH"
        "g6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUg"
        "Ni4wLjAiPgogICA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OT"
        "kvMDIvMjItcmRmLXN5bnRheC1ucyMiPgogICAgICA8cmRmOkRlc2NyaXB0aW9uIHJkZjph"
        "Ym91dD0iIgogICAgICAgICAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3"
        "RpZmYvMS4wLyI+"
        "CiAgICAgICAgIDx0aWZmOk9yaWVudGF0aW9uPjE8L3RpZmY6T3JpZW50YXRpb24+"
        "CiAgICAgIDwvcmRmOkRlc2NyaXB0aW9uPgogICA8L3JkZjpSREY+"
        "CjwveDp4bXBtZXRhPgoZXuEHAAADOElEQVQ4EY1USUtbURQ+N+/"
        "Fh1O04IxYHFAQNKiIIOJGV10oVbEb3fgHJLpKf0DjQuim3RS3roR25c7qzoWgkCAiiAM4I"
        "W5MnPUlt+c75j6iCdILN+/cM3xnzFFEpPjqYDD4WWsd4htUSgXAS8v4k3VExroJ1o3y/"
        "R6NRv+wlrKg2t7e/s2yrB+2bX8sKChwwHNdl/"
        "Xg6+WwMTmOQ+Alk0mR+Xw+Bzb8+FJRUeFcXFz8VYiMBb/ZzE0kEnp/"
        "f99mWnV2dtLz87MAAMzv99PR0RGVlJRQaWkpQCkvL49F2oV+KpWy+Y5YlZWVv+"
        "Dl6uoq2dra6p+enlYtLS20vr4uhqwkYCcnJzQ0NCQ8dkqFhYW0tbWlzs/"
        "PrcfHx2RZWZnFAdRQW1tbvLe3FzVJzc/Pw6OOxWJ4a/C5HLqvr0/"
        "ex8fHenV1VWjIFxcX9cHBgR4YGEg1Nzfrjo6OuM35BwCCsPnKQTo4SJNrQysrKzQxMUG1t"
        "bVSRxGm5aDZXBrLZMD3FgwK5uzs7AhYXV0dhUIhYZeXl9PS0pLQ4+"
        "Pj1NDQQLu7u1RUVKS4kdrHEi8yA4RO4kxNTdHm5iZtb28TmvSSCNHY2BhdXl7S2toajY6O"
        "UiAQINQarfcZYwOGr+"
        "FVV1dTU1MTFRcXe2IDyk2gxsZG6TqPC3FjRQcRZh14w5mZmRGDcDhMp6en4gjOuLs0OTlJ"
        "KMXy8jLV19cTd1psXmCFzP7p7++XVObm5kQYiUTo+"
        "vqahoeHCWPE3ae7uztvXqGUM0IDDa83NzcEYIDy4NPe3p6ADQ4Oypsb4ZUIdrapiQHJ/"
        "CI9GDw8PAh7YWGBzs7OhI7H45mqHo2UX82gJ0kTAEVa3d3dNDs7Kw3q6uoSJ6Z5GTYag22"
        "GMmt8TPT8XxeAnp4e+Q+jFLnAGFghZaygV+uKN484hZEBBX1/f+/"
        "xM6ICqVmOBZGwqqqqPrHRx/z8fPf29tbiJUH8f6XDw0PiVSZdfmOc6+lyFohi47/"
        "WVy6ENA/1l/"
        "XFg23zDhiRuqUXbBi1whJ9enqSQUWa7x3IcWHH0xDhLfUVYSpsWt6LMfZQwwX/"
        "wLVwWPG97osM9Wf7Df6GGOwnsP4BQFiPuOZ8wJUAAAAASUVORK5CYII=");
    std::cout << "Tray ID: " << tray_icon.id << std::endl;
    std::cout << "Tray Title: " << tray_icon.GetTitle() << std::endl;

    // Create context menu
    auto context_menu = Menu::Create();

    // Add menu items
    auto show_window_item = MenuItem::Create("Show Window", MenuItemType::Normal);
    show_window_item->AddListener<MenuItemSelectedEvent>([window_ptr](const MenuItemSelectedEvent& event) {
      std::cout << "Show Window clicked from context menu" << std::endl;
      if (window_ptr) {
        window_ptr->Show();
        window_ptr->Focus();
      }
    });
    context_menu->AddItem(show_window_item);

    auto hide_window_item = MenuItem::Create("Hide Window", MenuItemType::Normal);
    hide_window_item->AddListener<MenuItemSelectedEvent>([window_ptr](const MenuItemSelectedEvent& event) {
      std::cout << "Hide Window clicked from context menu" << std::endl;
      if (window_ptr) {
        window_ptr->Hide();
      }
    });
    context_menu->AddItem(hide_window_item);

    // Add separator
    context_menu->AddSeparator();

    // Add about item
    auto about_item = MenuItem::Create("About", MenuItemType::Normal);
    about_item->AddListener<MenuItemSelectedEvent>([](const MenuItemSelectedEvent& event) {
      std::cout << "About clicked from context menu" << std::endl;
      std::cout << "Window Example v1.0 - Native API Demo" << std::endl;
    });
    context_menu->AddItem(about_item);

    // Add another separator
    context_menu->AddSeparator();

    // Add exit item
    auto exit_item = MenuItem::Create("Exit", MenuItemType::Normal);
    exit_item->AddListener<MenuItemSelectedEvent>([&window_manager](const MenuItemSelectedEvent& event) {
      std::cout << "Exit clicked from context menu" << std::endl;
      // Get all windows and destroy them to trigger app exit
      auto windows = window_manager.GetAll();
      for (auto& window : windows) {
        window_manager.Destroy(window->id);
      }
    });
    context_menu->AddItem(exit_item);

    // Set the context menu to the tray icon
    tray_icon.SetContextMenu(context_menu);

    // Set up click handlers
    tray_icon.SetOnLeftClick([]() {
      std::cout << "*** TRAY ICON LEFT CLICKED! ***" << std::endl;
      std::cout << "This is the left click handler working!" << std::endl;
    });

    tray_icon.SetOnRightClick([&tray_icon]() {
      std::cout << "*** TRAY ICON RIGHT CLICKED! ***" << std::endl;
      std::cout << "This is the right click handler working!" << std::endl;
      // Context menu will be shown automatically on right-click
      // But we can also manually show it if needed:
      // tray_icon.ShowContextMenu();
    });

    tray_icon.SetOnDoubleClick([]() {
      std::cout << "*** TRAY ICON DOUBLE CLICKED! ***" << std::endl;
      std::cout << "This is the double click handler working!" << std::endl;
    });
  } else {
    std::cerr << "Failed to create tray." << std::endl;
  }

  display_manager.AddListener<nativeapi::DisplayAddedEvent>(
      [](const nativeapi::DisplayAddedEvent& event) {
        std::cout << "Display added: " << event.GetDisplay().id << std::endl;
      });
  display_manager.AddListener<nativeapi::DisplayRemovedEvent>(
      [](const nativeapi::DisplayRemovedEvent& event) {
        std::cout << "Display removed: " << event.GetDisplay().id << std::endl;
      });

  RunApp(window_ptr);

  return 0;
}
