#include <iostream>
#include "nativeapi.h"

using nativeapi::AppRunner;
using nativeapi::Display;
using nativeapi::DisplayAddedEvent;
using nativeapi::DisplayManager;
using nativeapi::DisplayRemovedEvent;
using nativeapi::Tray;
using nativeapi::TrayManager;
using nativeapi::Window;
using nativeapi::WindowManager;
using nativeapi::WindowOptions;

int main() {
  DisplayManager display_manager = DisplayManager();
  TrayManager tray_manager = TrayManager();
  WindowManager window_manager = WindowManager();

  // Create a new window with options
  WindowOptions options = {.title = "Window Example",
                           .size = {800, 600},
                           .minimum_size = {400, 300},
                           .maximum_size = {1920, 1080},
                           .centered = true};
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

  display_manager.AddListener<nativeapi::DisplayAddedEvent>(
      [](const nativeapi::DisplayAddedEvent& event) {
        std::cout << "Display added: " << event.GetDisplay().id << std::endl;
      });
  display_manager.AddListener<nativeapi::DisplayRemovedEvent>(
      [](const nativeapi::DisplayRemovedEvent& event) {
        std::cout << "Display removed: " << event.GetDisplay().id << std::endl;
      });

  AppRunner runner;
  runner.Run(window_ptr);

  return 0;
}
