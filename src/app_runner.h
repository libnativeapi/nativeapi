#pragma once

#include <functional>
#include <memory>

#include "window.h"

namespace nativeapi {

/**
 * AppRunner is a class that manages the application lifecycle and runs the main
 * event loop. It provides a way to run an application with a given window and
 * handle application events.
 */
class AppRunner {
 public:
  AppRunner();
  virtual ~AppRunner();

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
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
