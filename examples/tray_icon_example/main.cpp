#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "../../src/menu.h"
#include "../../src/tray_icon.h"
#include "../../src/tray_icon_event.h"
#include "../../src/tray_manager.h"

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif

using namespace nativeapi;
using nativeapi::Menu;
using nativeapi::MenuItem;
using nativeapi::MenuItemClickedEvent;
using nativeapi::MenuItemType;

int main() {
  std::cout << "Starting TrayIcon Example..." << std::endl;

#ifdef __APPLE__
  // Initialize Cocoa application for macOS
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
#endif

  // Check if tray icons are supported
  TrayManager& trayManager = TrayManager::GetInstance();
  if (!trayManager.IsSupported()) {
    std::cerr << "Tray icons are not supported on this platform!" << std::endl;
    return 1;
  }

  // Create a tray icon directly
  auto trayIcon = std::make_shared<TrayIcon>();
  if (!trayIcon) {
    std::cerr << "Failed to create tray icon!" << std::endl;
    return 1;
  }

  // Set up the tray icon
  trayIcon->SetTitle("Test App");
  trayIcon->SetTooltip("This is a test tray icon");

  // Try to set a system icon (using a system-provided icon)
  trayIcon->SetIcon("NSImageNameStatusAvailable");

  // Set up event listeners
  trayIcon->AddListener<TrayIconClickedEvent>(
      [](const TrayIconClickedEvent& event) {
        std::cout << "*** TRAY ICON LEFT CLICKED! ***" << std::endl;
        std::cout << "This is the left click handler working!" << std::endl;
        std::cout << "Tray icon ID: " << event.GetTrayIconId() << std::endl;
      });

  trayIcon->AddListener<TrayIconRightClickedEvent>(
      [](const TrayIconRightClickedEvent& event) {
        std::cout << "*** TRAY ICON RIGHT CLICKED! ***" << std::endl;
        std::cout << "This is the right click handler working!" << std::endl;
        std::cout << "Tray icon ID: " << event.GetTrayIconId() << std::endl;
        // Context menu will be shown automatically
      });

  trayIcon->AddListener<TrayIconDoubleClickedEvent>(
      [](const TrayIconDoubleClickedEvent& event) {
        std::cout << "*** TRAY ICON DOUBLE CLICKED! ***" << std::endl;
        std::cout << "This is the double click handler working!" << std::endl;
        std::cout << "Tray icon ID: " << event.GetTrayIconId() << std::endl;
      });

  // Create context menu
  auto context_menu = std::make_shared<Menu>();

  // Add menu items
  auto status_item =
      std::make_shared<MenuItem>("Status: Running", MenuItemType::Normal);
  status_item->AddListener<MenuItemClickedEvent>(
      [](const MenuItemClickedEvent& event) {
        std::cout << "Status clicked from context menu" << std::endl;
      });
  context_menu->AddItem(status_item);

  // Add separator
  context_menu->AddSeparator();

  // Add settings item
  auto settings_item =
      std::make_shared<MenuItem>("Settings...", MenuItemType::Normal);
  settings_item->AddListener<MenuItemClickedEvent>(
      [](const MenuItemClickedEvent& event) {
        std::cout << "Settings clicked from context menu" << std::endl;
        std::cout << "Opening settings dialog..." << std::endl;
      });
  context_menu->AddItem(settings_item);

  // Add about item
  auto about_item = std::make_shared<MenuItem>("About", MenuItemType::Normal);
  about_item->AddListener<MenuItemClickedEvent>(
      [](const MenuItemClickedEvent& event) {
        std::cout << "About clicked from context menu" << std::endl;
        std::cout << "TrayIcon Example v1.0 - Native API Demo" << std::endl;
      });
  context_menu->AddItem(about_item);

  // Add another separator
  context_menu->AddSeparator();

  // Add exit item
  auto exit_item = std::make_shared<MenuItem>("Exit", MenuItemType::Normal);
  bool* should_exit = new bool(false);
  exit_item->AddListener<MenuItemClickedEvent>(
      [should_exit](const MenuItemClickedEvent& event) {
        std::cout << "Exit clicked from context menu" << std::endl;
        *should_exit = true;
      });
  context_menu->AddItem(exit_item);

  // Set the context menu to the tray icon
  trayIcon->SetContextMenu(context_menu);

  // Show the tray icon
  if (trayIcon->SetVisible(true)) {
    std::cout << "Tray icon is now visible!" << std::endl;
  } else {
    std::cerr << "Failed to show tray icon!" << std::endl;
    return 1;
  }

  // Get and display bounds
  Rectangle bounds = trayIcon->GetBounds();
  std::cout << "Tray icon bounds: x=" << bounds.x << ", y=" << bounds.y
            << ", width=" << bounds.width << ", height=" << bounds.height
            << std::endl;

  std::cout << "========================================" << std::endl;
  std::cout << "Tray icon example is now running!" << std::endl;
  std::cout << "Try clicking on the tray icon:" << std::endl;
  std::cout << "- Left click: Single click" << std::endl;
  std::cout << "- Right click: Opens context menu" << std::endl;
  std::cout << "- Double click: Quick double click" << std::endl;
  std::cout << "- Context menu: Right-click to see options including Exit"
            << std::endl;
  std::cout
      << "The application will run for 60 seconds, or until you click Exit."
      << std::endl;
  std::cout << "========================================" << std::endl;

  // Keep the application running for 60 seconds or until exit is clicked
  int countdown = 60;
  while (countdown > 0 && !*should_exit) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    countdown--;

    // Check if tray icon is still visible
    if (!trayIcon->IsVisible()) {
      std::cout << "Tray icon is no longer visible!" << std::endl;
      break;
    }

    // Print countdown every 10 seconds
    if (countdown % 10 == 0) {
      std::cout << "Application will exit in " << countdown << " seconds..."
                << std::endl;
    }
  }

  if (*should_exit) {
    std::cout << "Exit requested from context menu." << std::endl;
  }

  // Hide the tray icon before exiting
  trayIcon->SetVisible(false);
  std::cout << "Exiting TrayIcon Example..." << std::endl;

  // Cleanup
  delete should_exit;
  return 0;
}