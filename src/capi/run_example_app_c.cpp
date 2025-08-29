#include <cstdlib>
#include <memory>
#include "run_example_app_c.h"

#include "../app_runner.h"
#include "../window_manager.h"

using namespace nativeapi;

int native_run_example_app() {
  // Create a new window with options
  WindowOptions options = {.title = "Window Example",
                           .size = {800, 600},
                           .minimum_size = {400, 300},
                           .maximum_size = {1920, 1080},
                           .centered = true};
  std::shared_ptr<Window> window_ptr =
      WindowManager::GetInstance().Create(options);
  // Run the main event loop
  int exit_code = RunApp(window_ptr);
  return exit_code;
}
