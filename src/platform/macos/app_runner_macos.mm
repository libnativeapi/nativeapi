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
  // Initialize NSApplication if not already done
  NSApplication* app = [NSApplication sharedApplication];

  // Set up the application delegate
  [app setDelegate:pimpl_->delegate];

  // If a window is provided, show it and make it the main window
  if (window) {
    window->Show();
  }

  // Activate the application
  [app activateIgnoringOtherApps:YES];

  // Set activation policy to regular app (appears in dock)
  [app setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Finish launching if not already done
  [app finishLaunching];

  // Start the main event loop
  [app run];

  return 0;
}

}  // namespace nativeapi
