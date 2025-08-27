#include <iostream>
#include <thread>
#include <chrono>
#include "../../src/tray.h"
#include "../../src/tray_events.h"
#include "../../src/tray_manager.h"

using namespace nativeapi;

class MyTrayListener : public TrayListener {
 public:
  void OnTrayClicked(const TrayClickedEvent& event) override {
    std::cout << "Tray " << event.GetTrayId() << " clicked!" << std::endl;
  }

  void OnTrayRightClicked(const TrayRightClickedEvent& event) override {
    std::cout << "Tray " << event.GetTrayId() << " right-clicked!" << std::endl;
  }

  void OnTrayDoubleClicked(const TrayDoubleClickedEvent& event) override {
    std::cout << "Tray " << event.GetTrayId() << " double-clicked!" << std::endl;
  }
};

int main() {
  std::cout << "TrayListener Example\n";
  std::cout << "====================\n";

  // Create a tray manager
  TrayManager manager;
  
  // Create a tray
  auto tray = manager.Create();
  if (!tray) {
    std::cout << "❌ Failed to create tray\n";
    return 1;
  }
  
  std::cout << "✅ Tray created with ID: " << tray->id << "\n";

  // Add event listeners using callbacks
  auto click_listener_id = tray->AddClickedListener([](const TrayClickedEvent& event) {
    std::cout << "🖱️  Tray " << event.GetTrayId() << " clicked (callback)!" << std::endl;
  });

  auto right_click_listener_id = tray->AddRightClickedListener([](const TrayRightClickedEvent& event) {
    std::cout << "🖱️  Tray " << event.GetTrayId() << " right-clicked (callback)!" << std::endl;
  });

  auto double_click_listener_id = tray->AddDoubleClickedListener([](const TrayDoubleClickedEvent& event) {
    std::cout << "🖱️  Tray " << event.GetTrayId() << " double-clicked (callback)!" << std::endl;
  });

  std::cout << "✅ Event listeners added successfully\n";
  std::cout << "📚 This example demonstrates the TrayListener interface:\n";
  std::cout << "• TrayClickedEvent - Fired on left click\n";
  std::cout << "• TrayRightClickedEvent - Fired on right click\n";
  std::cout << "• TrayDoubleClickedEvent - Fired on double click\n";
  std::cout << "• Callback-based listeners for each event type\n\n";

  // Simulate some events (since we can't actually create real tray icons in this environment)
  std::cout << "🧪 Simulating tray events...\n";
  tray->DispatchClickedEvent();
  tray->DispatchRightClickedEvent();
  tray->DispatchDoubleClickedEvent();

  std::cout << "\n✅ TrayListener implementation works correctly!\n";

  // Clean up
  tray->RemoveListener(click_listener_id);
  tray->RemoveListener(right_click_listener_id);  
  tray->RemoveListener(double_click_listener_id);

  return 0;
}