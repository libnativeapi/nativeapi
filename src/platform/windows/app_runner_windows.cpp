#include <windows.h>
#include "../../app_runner.h"

namespace nativeapi {

// Forward declaration of the implementation class
class AppRunner::Impl {
 public:
  Impl() = default;
  ~Impl() = default;

  int Run(std::shared_ptr<Window> window) {
    if (!window) {
      return -1;
    }

    // Show the window
    window->Show();

    // Get the HWND for message loop filtering (optional)
    HWND hwnd = static_cast<HWND>(window->GetNativeObject());

    // Start the Windows message loop
    MSG msg = {};
    int exit_code = 0;

    while (true) {
      // Get message from the message queue
      BOOL result = GetMessage(&msg, nullptr, 0, 0);

      if (result == -1) {
        // Error occurred
        exit_code = -1;
        break;
      } else if (result == 0) {
        // WM_QUIT received
        exit_code = static_cast<int>(msg.wParam);
        break;
      } else {
        // Translate and dispatch the message
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    return exit_code;
  }
};

AppRunner::AppRunner() : pimpl_(std::make_unique<Impl>()) {}

AppRunner::~AppRunner() = default;

AppRunner& AppRunner::GetInstance() {
  static AppRunner instance;
  return instance;
}

int AppRunner::Run(std::shared_ptr<Window> window) {
  return pimpl_->Run(window);
}

int RunApp(std::shared_ptr<Window> window) {
  return AppRunner::GetInstance().Run(window);
}

}  // namespace nativeapi
