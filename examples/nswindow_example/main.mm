#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#include <iostream>
#include "nativeapi.h"

using nativeapi::BroadcastCenter;
using nativeapi::BroadcastEventHandler;
using nativeapi::Display;
using nativeapi::DisplayEventHandler;
using nativeapi::DisplayManager;
using nativeapi::Tray;
using nativeapi::TrayManager;
using nativeapi::Window;
using nativeapi::WindowManager;

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property(strong) NSWindow* window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  // Create a window
  NSRect frame = NSMakeRect(0, 0, 400, 300);
  self.window = [[NSWindow alloc]
      initWithContentRect:frame
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                          NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                  backing:NSBackingStoreBuffered
                    defer:NO];
  [self.window setTitle:@"Native API Example"];
  [self.window center];
  [self.window makeKeyAndOrderFront:nil];
  [self.window makeMainWindow];
  [NSApp activateIgnoringOtherApps:YES];

  // 延迟检查主窗口
  dispatch_async(dispatch_get_main_queue(), ^{
    NSLog(@"主窗口: %@", NSApp.mainWindow);
    NSLog(@"关键窗口: %@", NSApp.keyWindow);
    NSLog(@"所有窗口: %@", NSApp.windows);
  });


  DisplayManager displayManager = DisplayManager();

  DisplayEventHandler displayEventHandler = DisplayEventHandler(
      [](const Display& display) {
        std::cout << "Display added: " << display.id << std::endl;
      },
      [](const Display& display) {
        std::cout << "Display removed: " << display.id << std::endl;
      });
  displayManager.AddListener(&displayEventHandler);

  std::shared_ptr<BroadcastCenter> broadcastCenter = std::make_shared<BroadcastCenter>();

  BroadcastEventHandler broadcastEventHandler =
      BroadcastEventHandler([](const std::string& message) {
        std::cout << "Received broadcast: " << message << std::endl;
      });

  broadcastCenter->RegisterReceiver("com.example.myNotification", &broadcastEventHandler);

  //  broadcastCenter.RegisterReceiver(
  //      BroadcastEventHandler ([&](const std::string& message) {
  //        std::cout << "Received broadcast: " << message << std::endl;
  //      }));

  [[NSNotificationCenter defaultCenter]
      addObserverForName:NSWindowDidBecomeMainNotification
                  object:nil
                   queue:nil
              usingBlock:^(NSNotification* note) {
                NSWindow* mainWindow = [[NSApplication sharedApplication] mainWindow];
                NSLog(@"主窗口: %@", mainWindow);

                // Initialize WindowManager
                WindowManager windowManager = WindowManager();

                // Get current window information
                std::shared_ptr<Window> currentWindowPtr = windowManager.GetCurrent();
                if (currentWindowPtr != nullptr) {
                  Window& currentWindow = *currentWindowPtr;
                  std::cout << "Current Window Information:" << std::endl;
                  std::cout << "ID: " << currentWindow.id << std::endl;

                  // Get window size
                  auto size = currentWindow.GetSize();
                  std::cout << "Window Size: " << size.width << "x" << size.height << std::endl;
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

                TrayManager trayManager = TrayManager();

                std::shared_ptr<Tray> newTrayPtr = trayManager.Create();
                if (newTrayPtr != nullptr) {
                  Tray& newTray = *newTrayPtr;
                  newTray.SetTitle("Hello, World!");
                  std::cout << "Tray ID: " << newTray.id << std::endl;
                  std::cout << "Tray Title: " << newTray.GetTitle() << std::endl;
                } else {
                  std::cerr << "Failed to create tray." << std::endl;
                }
              }];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
  return YES;
}

@end

int main(int argc, const char* argv[]) {
  @autoreleasepool {
    NSApplication* app = [NSApplication sharedApplication];
    AppDelegate* delegate = [[AppDelegate alloc] init];
    [app setDelegate:delegate];
    [app run];
  }
  return 0;
}