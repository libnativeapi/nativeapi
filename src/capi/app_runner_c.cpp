#include "app_runner_c.h"
#include <atomic>
#include <memory>

#include "../app_runner.h"

using namespace nativeapi;

// Global state for the app runner
static std::atomic<bool> g_is_running{false};
static std::atomic<int> g_exit_code{0};

// Helper to convert native_window_t back to shared_ptr<Window>
static std::shared_ptr<Window> GetWindowFromHandle(
    native_window_t window_handle) {
  if (!window_handle) {
    return nullptr;
  }

  // Assuming native_window_handle structure from window_c.cpp
  struct native_window_handle {
    std::shared_ptr<Window> window;
  };

  auto* handle = reinterpret_cast<native_window_handle*>(window_handle);
  return handle->window;
}

extern "C" {

int native_app_runner_run(native_window_t window) {
  // Validate input
  if (!window) {
    return NATIVE_APP_EXIT_INVALID_WINDOW;
  }

  auto window_ptr = GetWindowFromHandle(window);
  if (!window_ptr) {
    return NATIVE_APP_EXIT_INVALID_WINDOW;
  }

  // Mark as running
  g_is_running.store(true);
  g_exit_code.store(0);

  try {
    // Get the AppRunner instance and run the application
    AppRunner& runner = AppRunner::GetInstance();

    // Run the main event loop
    int exit_code = runner.Run(window_ptr);

    // Mark as no longer running
    g_is_running.store(false);

    // Use the stored exit code if terminate was called, otherwise use the
    // returned code
    int final_exit_code = g_exit_code.load();
    if (final_exit_code == 0) {
      final_exit_code = exit_code;
    }

    return final_exit_code;

  } catch (const std::exception& e) {
    // Handle any exceptions that might occur
    g_is_running.store(false);

    return NATIVE_APP_EXIT_FAILURE;
  }
}

bool native_app_runner_is_running(void) {
  return g_is_running.load();
}

}  // extern "C"
