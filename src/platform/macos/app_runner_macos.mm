#import <Cocoa/Cocoa.h>
#include <iostream>

#include "../../app_runner.h"
#include "../../window.h"

@interface AppRunnerDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppRunnerDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  NSLog(@"Application finished launching");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
  return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  return NSTerminateNow;
}

@end

namespace nativeapi {

// Private implementation class
class AppRunner::Impl {
 public:
  AppRunnerDelegate* delegate;

  Impl() { delegate = [[AppRunnerDelegate alloc] init]; }

  ~Impl() { delegate = nil; }
};

AppRunner::AppRunner() : pimpl_(new Impl()) {}

AppRunner::~AppRunner() {}

int AppRunner::Run(std::shared_ptr<Window> window) {
  std::cout << "Starting macOS application..." << std::endl;

  // Initialize NSApplication if not already done
  NSApplication* app = [NSApplication sharedApplication];

  // Set up the application delegate
  [app setDelegate:pimpl_->delegate];

  // If a window is provided, show it and make it the main window
  if (window) {
    // Access the NSWindow from the Window object
    // We need to get the native window handle
    window->Show();
    window->Focus();

    // Get the NSWindow from the Window object
    // This assumes the Window class has access to the native NSWindow
    std::cout << "Window shown and focused" << std::endl;
  }

  // Activate the application
  [app activateIgnoringOtherApps:YES];

  // Set activation policy to regular app (appears in dock)
  [app setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Finish launching if not already done
  [app finishLaunching];

  std::cout << "Starting main run loop..." << std::endl;

  // Start the main event loop
  [app run];

  std::cout << "Application event loop ended" << std::endl;

  return 0;
}

}  // namespace nativeapi
