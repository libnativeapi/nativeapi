#include <iostream>
#include "nativeapi.h"

using nativeapi::AppRunner;
using nativeapi::Tray;
using nativeapi::TrayManager;
using nativeapi::Window;
using nativeapi::WindowManager;
using nativeapi::WindowOptions;

int main() {
  WindowManager window_manager = WindowManager();

  // Create a new window with options
  WindowOptions options;
  options.title = "My Window";
  options.size.width = 800;
  options.size.height = 600;
  std::shared_ptr<Window> window_ptr = window_manager.Create(options);
  if (window_ptr != nullptr) {
    Window& window = *window_ptr;
    std::cout << "New Window Information:" << std::endl;
    std::cout << "ID: " << window.id << std::endl;
    std::cout << std::endl;
    window.Show();
    window.Focus();
  }

  TrayManager trayManager = TrayManager();

  std::shared_ptr<Tray> newTrayPtr = trayManager.Create();
  if (newTrayPtr != nullptr) {
    Tray& newTray = *newTrayPtr;
    newTray.SetTitle("Hello, World!");
    std::cout << "Tray ID: " << newTray.id << std::endl;
    std::cout << "Tray Title: " << newTray.GetTitle() << std::endl;
  } else {
    std::cerr << "Failed to create tray." << std::endl;
  }

  AppRunner runner;
  runner.Run(window_ptr);

  return 0;
}
