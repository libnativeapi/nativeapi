#include <iostream>
#include "nativeapi.h"

using nativeapi::Window;
using nativeapi::WindowManager;

int main() {
  WindowManager windowManager = WindowManager();

  // Get primary display information
  std::shared_ptr<Window> currentWindowPtr = windowManager.GetCurrent();
  if (currentWindowPtr != nullptr) {
    Window& currentWindow = *currentWindowPtr;
    std::cout << "Current Window Information:" << std::endl;
    std::cout << "ID: " << currentWindow.id << std::endl;
    std::cout << std::endl;
  }

  // Get all windows
  std::vector<std::shared_ptr<Window>> windowList = (windowManager.GetAll());
  std::cout << "\nAll Windows Information:" << std::endl;
  for (size_t i = 0; i < windowList.size(); i++) {
    const Window& window = *windowList[i];
    std::cout << "Window " << (i + 1) << ":" << std::endl;
    std::cout << "ID: " << window.id << std::endl;
    auto windowSize = window.GetSize();
    std::cout << "Size: " << windowSize.width << "x" << windowSize.height
              << std::endl;
  }
  return 0;
}