#include <iostream>
#include "nativeapi.h"

using nativeapi::AppRunner;
using nativeapi::Tray;
using nativeapi::TrayManager;
using nativeapi::Window;
using nativeapi::WindowManager;
using nativeapi::WindowOptions;

int main() {
  TrayManager tray_manager = TrayManager();
  WindowManager window_manager = WindowManager();

  // Create a new window with options
  WindowOptions options;
  options.title = "My Window";
  options.size.width = 800;
  options.size.height = 600;
  std::shared_ptr<Window> window_ptr = window_manager.Create(options);


  std::shared_ptr<Tray> tray_ptr = tray_manager.Create();
  if (tray_ptr != nullptr) {
    Tray& tray = *tray_ptr;
    tray.SetTitle("Hello, World!");
    std::cout << "Tray ID: " << tray.id << std::endl;
    std::cout << "Tray Title: " << tray.GetTitle() << std::endl;
  } else {
    std::cerr << "Failed to create tray." << std::endl;
  }

  AppRunner runner;
  runner.Run(window_ptr);

  return 0;
}
