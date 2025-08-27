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

  // Set position and size using geometry types
  display.position = {frame.origin.x, frame.origin.y};
  display.size = {frame.size.width, frame.size.height};
  display.workArea = {visibleFrame.origin.x, visibleFrame.origin.y,
                     visibleFrame.size.height, visibleFrame.size.width};
  display.scaleFactor = scaleFactor;
  display.isPrimary = isPrimary;

  // Determine orientation based on dimensions
  if (frame.size.width > frame.size.height) {
    display.orientation = DisplayOrientation::kLandscape;
  } else {
    display.orientation = DisplayOrientation::kPortrait;
  }

  // Try to get refresh rate using Core Graphics
  CGDisplayModeRef displayMode = CGDisplayCopyDisplayMode(displayID);
  if (displayMode) {
    double refreshRate = CGDisplayModeGetRefreshRate(displayMode);
    display.refreshRate = refreshRate > 0 ? (int)refreshRate : 60; // Default to 60Hz if unknown
    CGDisplayModeRelease(displayMode);
  }

  // Set default bit depth for modern displays
  display.bitDepth = 32;

  // Try to get hardware info using newer APIs
  // For now, use display name parsing and default values since the old API is deprecated
  // This could be enhanced with newer IOKit approaches if needed
  NSString* displayNameStr = [NSString stringWithUTF8String:display.name.c_str()];
  if ([displayNameStr containsString:@"Apple"]) {
    display.manufacturer = "Apple";
  } else if ([displayNameStr containsString:@"Dell"]) {
    display.manufacturer = "Dell";
  } else if ([displayNameStr containsString:@"Samsung"]) {
    display.manufacturer = "Samsung";
  } else if ([displayNameStr containsString:@"LG"]) {
    display.manufacturer = "LG";
  } else {
    display.manufacturer = "Unknown";
  }

  // Use display name as model for now
  display.model = display.name;
  display.serialNumber = ""; // Not easily available without deprecated APIs

  // Set default values if hardware info couldn't be retrieved
  if (display.manufacturer.empty()) display.manufacturer = "Unknown";
  if (display.model.empty()) display.model = "Unknown";

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
                    DispatchDisplayAddedEvent(d);
                  }
                }

                // Find removed displays
                std::set<std::string> new_ids;
                for (const auto& d : new_displays)
                  new_ids.insert(d.id);
                for (const auto& d : old_displays) {
                  if (new_ids.find(d.id) == new_ids.end()) {
                    DispatchDisplayRemovedEvent(d);
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
