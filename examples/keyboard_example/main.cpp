#include <signal.h>
#include <chrono>
#include <iostream>
#include <thread>

extern "C" {
#include "../../src/capi/keyboard_monitor_c.h"
}

// Global monitor handle for cleanup
static native_keyboard_monitor_t* g_monitor = nullptr;
static bool g_running = true;

// Signal handler for graceful shutdown
void signal_handler(int sig) {
  std::cout << "\nReceived signal " << sig << ", shutting down...\n";
  g_running = false;
  if (g_monitor) {
    native_keyboard_monitor_stop(g_monitor);
    native_keyboard_monitor_destroy(g_monitor);
    g_monitor = nullptr;
  }
  exit(0);
}

// Callback for key pressed events
void on_key_pressed(int keycode, void* user_data) {
  std::cout << "Key pressed: " << keycode << std::endl;
}

// Callback for key released events
void on_key_released(int keycode, void* user_data) {
  std::cout << "Key released: " << keycode << std::endl;
}

// Callback for modifier keys changed events
void on_modifier_keys_changed(uint32_t modifier_keys, void* user_data) {
  std::cout << "Modifier keys changed: 0x" << std::hex << modifier_keys << std::dec;

  if (modifier_keys & NATIVE_MODIFIER_KEY_SHIFT)
    std::cout << " SHIFT";
  if (modifier_keys & NATIVE_MODIFIER_KEY_CTRL)
    std::cout << " CTRL";
  if (modifier_keys & NATIVE_MODIFIER_KEY_ALT)
    std::cout << " ALT";
  if (modifier_keys & NATIVE_MODIFIER_KEY_META)
    std::cout << " META";
  if (modifier_keys & NATIVE_MODIFIER_KEY_FN)
    std::cout << " FN";
  if (modifier_keys & NATIVE_MODIFIER_KEY_CAPS_LOCK)
    std::cout << " CAPS";
  if (modifier_keys & NATIVE_MODIFIER_KEY_NUM_LOCK)
    std::cout << " NUM";
  if (modifier_keys & NATIVE_MODIFIER_KEY_SCROLL_LOCK)
    std::cout << " SCROLL";

  std::cout << std::endl;
}

int main() {
  std::cout << "KeyboardMonitor C API Example\n";
  std::cout << "==============================\n";

  // Set up signal handlers
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // Create keyboard monitor
  g_monitor = native_keyboard_monitor_create();
  if (!g_monitor) {
    std::cout << "Failed to create keyboard monitor\n";
    return 1;
  }
  std::cout << "Keyboard monitor created successfully\n";

  // Set up callbacks
  if (!native_keyboard_monitor_set_callbacks(g_monitor, on_key_pressed, on_key_released,
                                             on_modifier_keys_changed, nullptr)) {
    std::cout << "Failed to set callbacks\n";
    native_keyboard_monitor_destroy(g_monitor);
    return 1;
  }
  std::cout << "Callbacks set successfully\n";

  // Start monitoring
  if (!native_keyboard_monitor_start(g_monitor)) {
    std::cout << "Failed to start keyboard monitoring\n";
    native_keyboard_monitor_destroy(g_monitor);
    return 1;
  }

  if (native_keyboard_monitor_is_monitoring(g_monitor)) {
    std::cout << "Keyboard monitoring is now active\n";
    std::cout << "\nThis example demonstrates the KeyboardMonitor C API:\n";
    std::cout << "• native_keyboard_monitor_create() - Creates a monitor instance\n";
    std::cout << "• native_keyboard_monitor_set_callbacks() - Sets event callbacks\n";
    std::cout << "• native_keyboard_monitor_start() - Starts monitoring\n";
    std::cout << "• native_keyboard_monitor_is_monitoring() - Checks status\n";
    std::cout << "• native_keyboard_monitor_stop() - Stops monitoring\n";
    std::cout << "• native_keyboard_monitor_destroy() - Cleans up resources\n";
    std::cout << "\nPress keys to see events. Press Ctrl+C to exit.\n\n";
  } else {
    std::cout << "Warning: Monitor created but not monitoring (may be due to "
                 "permissions or display server)\n";
    std::cout << "This is expected in headless environments or without proper "
                 "permissions.\n";
    std::cout << "On a desktop system with X11/Wayland, you would see keyboard "
                 "events.\n\n";
  }

  // Keep the main thread alive to receive events
  while (g_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}