#include "run_example_app_c.h"
#include <cstdlib>
#include <memory>

#include "../application.h"
#include "../window_manager.h"

using namespace nativeapi;

int native_run_example_app() {
  // Create a new window with options
  WindowOptions options;
  options.title = "Window Example";
  options.size = {800, 600};
  options.minimum_size = {400, 300};
  options.maximum_size = {1920, 1080};
  options.centered = true;
  std::shared_ptr<Window> window_ptr =
      WindowManager::GetInstance().Create(options);
  // Run the main event loop
  auto& app = Application::GetInstance();
  int exit_code = app.Run(window_ptr);
  return exit_code;
}
