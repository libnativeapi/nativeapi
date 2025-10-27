#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#include "../../display_manager.h"
#include "../../window.h"

namespace nativeapi {

DisplayManager::DisplayManager() {
  // Constructor initialization
}

DisplayManager::~DisplayManager() {}

std::vector<Display> DisplayManager::GetAll() {
  std::vector<Display> displays;
  
  // Get all screens
  NSArray<UIScreen*>* screens = [UIScreen screens];
  for (UIScreen* screen in screens) {
    displays.push_back(Display((__bridge void*)screen));
  }
  
  // If no screens found, add main screen
  if (displays.empty()) {
    UIScreen* mainScreen = [UIScreen mainScreen];
    if (mainScreen) {
      displays.push_back(Display((__bridge void*)mainScreen));
    }
  }
  
  return displays;
}

Display DisplayManager::GetPrimary() {
  // Get main screen
  UIScreen* mainScreen = [UIScreen mainScreen];
  return Display((__bridge void*)mainScreen);
}

Point DisplayManager::GetCursorPosition() {
  // iOS doesn't have a cursor position concept
  return Point{0, 0};
}

}  // namespace nativeapi

