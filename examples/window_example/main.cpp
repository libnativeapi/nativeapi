#include <iostream>
#include "nativeapi.h"

using nativeapi::Window;
using nativeapi::WindowManager;

int main() {
  WindowManager windowManager = WindowManager();

  windowManager.Create();

  // Get primary display information
  Window currentWindow = windowManager.GetCurrent();
  std::cout << "Current Window Information:" << std::endl;
  std::cout << "ID: " << currentWindow.id << std::endl;
  std::cout << "Name: " << currentWindow.name << std::endl;
  std::cout << std::endl;

  // Get all displays information
  std::vector<Window> windowList = windowManager.GetAll();
  std::cout << "All Windows Information:" << std::endl;
  for (int i = 0; i < windowList.size(); i++) {
    Window& window = windowList[i];
    std::cout << "Window " << (i + 1) << ":" << std::endl;
    std::cout << "ID: " << window.id << std::endl;
    std::cout << "Name: " << window.name << std::endl;
    std::cout << std::endl;
  }
  return 0;
}