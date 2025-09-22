#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../../display.h"
#include "../../display_manager.h"

// Import Cocoa and Core Graphics headers
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

namespace nativeapi {

static Display CreateDisplayFromNSScreen(NSScreen* screen, bool isPrimary) {
  // Simply create Display with NSScreen - all properties will be read directly from the screen
  return Display((__bridge void*)screen);
}

id displayObserver_;

DisplayManager::DisplayManager() {
  displays_ = GetAll();
  // Set up display configuration change observer
  displayObserver_ = [[NSNotificationCenter defaultCenter]
      addObserverForName:NSApplicationDidChangeScreenParametersNotification
                  object:nil
                   queue:[NSOperationQueue mainQueue]
              usingBlock:^(NSNotification* notification) {
                auto old_displays = displays_;
                auto new_displays = GetAll();

                // Find added displays
                std::set<std::string> old_ids;
                for (const auto& d : old_displays)
                  old_ids.insert(d.GetId());
                for (const auto& d : new_displays) {
                  if (old_ids.find(d.GetId()) == old_ids.end()) {
                    EmitSync<DisplayAddedEvent>(d);
                  }
                }

                // Find removed displays
                std::set<std::string> new_ids;
                for (const auto& d : new_displays)
                  new_ids.insert(d.GetId());
                for (const auto& d : old_displays) {
                  if (new_ids.find(d.GetId()) == new_ids.end()) {
                    EmitSync<DisplayRemovedEvent>(d);
                  }
                }

                displays_ = std::move(new_displays);
              }];
}

DisplayManager::~DisplayManager() {
  if (displayObserver_) {
    [[NSNotificationCenter defaultCenter] removeObserver:displayObserver_];
  }
}

std::vector<Display> DisplayManager::GetAll() {
  std::vector<Display> displayList;
  NSArray<NSScreen*>* screens = [NSScreen screens];
  bool isPrimary = true;
  for (NSScreen* screen in screens) {
    displayList.push_back(CreateDisplayFromNSScreen(screen, isPrimary));
    isPrimary = false;  // Only the first NSScreen is the primary display
  }
  return displayList;
}

Display DisplayManager::GetPrimary() {
  // Get the primary display (first NSScreen)
  NSArray<NSScreen*>* screens = [NSScreen screens];
  return CreateDisplayFromNSScreen(screens[0], true);
}

Point DisplayManager::GetCursorPosition() {
  NSPoint mouseLocation = [NSEvent mouseLocation];
  Point point;
  point.x = mouseLocation.x;
  point.y = mouseLocation.y;
  return point;
}

}  // namespace nativeapi
