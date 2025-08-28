#pragma once

#include <functional>
#include <memory>

#include "window.h"

namespace nativeapi {

/**
 * AppRunner is a singleton class that manages the application lifecycle and runs the main
 * event loop. It provides a way to run an application with a given window and
 * handle application events.
 */
class AppRunner {
 public:
  /**
   * Gets the singleton instance of AppRunner.
   * @return Reference to the singleton AppRunner instance
   */
  static AppRunner& GetInstance();

  // Delete copy constructor and assignment operator
  AppRunner(const AppRunner&) = delete;
  AppRunner& operator=(const AppRunner&) = delete;

  /**
   * Runs the application with the specified window.
   * This method starts the main event loop and blocks until the application
   * exits.
   *
   * @param window The window to run the application with
   * @return Exit code of the application (0 for success)
   */
  int Run(std::shared_ptr<Window> window);


 private:
  AppRunner();
  virtual ~AppRunner();

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

/**
 * Convenience function to run the application with the specified window.
 * This is equivalent to calling AppRunner::GetInstance().Run(window).
 *
 * @param window The window to run the application with
 * @return Exit code of the application (0 for success)
 */
int RunApp(std::shared_ptr<Window> window);

}  // namespace nativeapi