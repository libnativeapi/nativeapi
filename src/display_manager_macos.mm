#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "display.h"
#include "display_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

static Display CreateDisplayFromNSScreen(NSScreen* screen, bool isPrimary) {
  Display display;

  // Get screen details
  NSRect frame = [screen frame];
  NSRect visibleFrame = [screen visibleFrame];
  CGFloat scaleFactor = [screen backingScaleFactor];

  // Set unique identifier for the screen using CGDirectDisplayID
  CGDirectDisplayID displayID =
      [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
  NSString* screenId = [NSString stringWithFormat:@"%@", @(displayID)];
  display.id = [screenId UTF8String];

  // Set display name - use localizedName on macOS 10.15+
  NSString* displayName;
  if (@available(macOS 10.15, *)) {
    displayName = [screen localizedName];
  } else {
    displayName =
        isPrimary ? @"Primary Display" : [NSString stringWithFormat:@"Display %@", @(displayID)];
  }
  display.name = [displayName UTF8String];

  // Set size and position properties
  display.width = frame.size.width;
  display.height = frame.size.height;
  display.visiblePositionX = visibleFrame.origin.x;
  display.visiblePositionY = visibleFrame.origin.y;
  display.visibleSizeWidth = visibleFrame.size.width;
  display.visibleSizeHeight = visibleFrame.size.height;
  display.scaleFactor = scaleFactor;

  return display;
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
                  old_ids.insert(d.id);
                for (const auto& d : new_displays) {
                  if (old_ids.find(d.id) == old_ids.end()) {
                    NotifyDisplayAdded(d);
                  }
                }

                // Find removed displays
                std::set<std::string> new_ids;
                for (const auto& d : new_displays)
                  new_ids.insert(d.id);
                for (const auto& d : old_displays) {
                  if (new_ids.find(d.id) == new_ids.end()) {
                    NotifyDisplayRemoved(d);
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

Point DisplayManager::GetCursorPosition() {
  NSPoint mouseLocation = [NSEvent mouseLocation];
  Point point;
  point.x = mouseLocation.x;
  point.y = mouseLocation.y;
  return point;
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

}  // namespace nativeapi
