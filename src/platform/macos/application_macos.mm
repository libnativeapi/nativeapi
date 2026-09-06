#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "../../application.h"
#include "../../menu.h"
#include "../../window_manager.h"

@interface NativeApplicationDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic, assign) nativeapi::Application* app;
@end

@implementation NativeApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  // Emit application started event
  nativeapi::ApplicationStartedEvent event;
  self.app->Emit(event);
}

- (void)applicationWillTerminate:(NSNotification*)notification {
  // Emit application exiting event
  nativeapi::ApplicationExitingEvent event(0);
  self.app->Emit(event);
}

- (void)applicationDidBecomeActive:(NSNotification*)notification {
  // Emit application activated event
  nativeapi::ApplicationActivatedEvent event;
  self.app->Emit(event);
}

- (void)applicationDidResignActive:(NSNotification*)notification {
  // Emit application deactivated event
  nativeapi::ApplicationDeactivatedEvent event;
  self.app->Emit(event);
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  // Emit quit requested event
  nativeapi::ApplicationQuitRequestedEvent event;
  self.app->Emit(event);

  // Allow termination
  return NSTerminateNow;
}

@end

namespace nativeapi {

class Application::Impl {
 public:
  Impl(Application* app) : app_(app), delegate_(nullptr) {}
  ~Impl() = default;

  bool Initialize() {
    // Ensure we're on the main thread
    if (![NSThread isMainThread]) {
      return false;
    }

    // Get or create NSApplication instance
    NSApplication* ns_app = [NSApplication sharedApplication];
    if (!ns_app) {
      return false;
    }

    // Set dock icon visible by default
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // Create and set delegate
    delegate_ = [[NativeApplicationDelegate alloc] init];
    delegate_.app = app_;
    [ns_app setDelegate:delegate_];

    return true;
  }

  int Run() {
    // Start the main event loop
    [NSApp run];

    return 0;
  }

  int Run(std::shared_ptr<Window> window) {
    if (!window) {
      return -1;
    }

    // Set the window as primary window
    app_->SetPrimaryWindow(window);

    // Show the window
    window->Show();
    window->Focus();

    // Start the main event loop
    [NSApp run];

    return 0;
  }

  void Quit(int exit_code) { [NSApp terminate:nil]; }

  bool SetIcon(const std::string& icon_path) {
    if (icon_path.empty()) {
      return false;
    }

    NSString* path = [NSString stringWithUTF8String:icon_path.c_str()];
    NSImage* icon = [[NSImage alloc] initWithContentsOfFile:path];

    if (!icon) {
      return false;
    }

    [NSApp setApplicationIconImage:icon];
    return true;
  }

  bool SetDockIconVisible(bool visible) {
    if (visible) {
      [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    } else {
      [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    }
    return true;
  }

  bool SetProgressBar(double progress) {
    NSDockTile* dock_tile = [NSApp dockTile];

    if (progress < 0) {
      // Hand the tile back to AppKit so it shows the plain application icon.
      dock_tile.contentView = nil;
      dock_icon_view_ = nil;
      dock_progress_ = nil;
      [dock_tile display];
      return true;
    }

    if (!dock_progress_) {
      dock_icon_view_ = [[NSImageView alloc] init];
      dock_tile.contentView = dock_icon_view_;

      NSRect frame = NSMakeRect(0, 0, dock_tile.size.width, 15.0);
      dock_progress_ = [[NSProgressIndicator alloc] initWithFrame:frame];
      dock_progress_.style = NSProgressIndicatorStyleBar;
      dock_progress_.minValue = 0;
      dock_progress_.maxValue = 1;
      [dock_icon_view_ addSubview:dock_progress_];
    }
    // Re-read the icon every time so a later SetIcon() shows through.
    dock_icon_view_.image = [NSApp applicationIconImage];

    if (progress > 1) {
      dock_progress_.indeterminate = YES;
      dock_progress_.doubleValue = 1;
      [dock_progress_ startAnimation:nil];
    } else {
      [dock_progress_ stopAnimation:nil];
      dock_progress_.indeterminate = NO;
      dock_progress_.doubleValue = progress;
    }
    [dock_tile display];
    return true;
  }

  bool SetBadgeLabel(const std::string& label) {
    NSDockTile* dock_tile = [NSApp dockTile];
    dock_tile.badgeLabel = label.empty() ? nil : [NSString stringWithUTF8String:label.c_str()];
    return true;
  }

  bool SetBrightness(Brightness brightness) {
    if (@available(macOS 10.14, *)) {
      NSAppearance* appearance = nil;
      switch (brightness) {
        case Brightness::Light:
          appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
          break;
        case Brightness::Dark:
          appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
          break;
        case Brightness::System:
        default:
          break;  // nil = inherit from the system
      }
      NSApp.appearance = appearance;
      return true;
    }
    // Dark mode does not exist before 10.14, so only "light" and "system" are
    // satisfiable and they are already in effect.
    return brightness != Brightness::Dark;
  }

  bool SetMenuBar(std::shared_ptr<Menu> menu) {
    if (!menu) {
      return false;
    }

    // Get the native menu handle
    NSMenu* ns_menu = (__bridge NSMenu*)(menu->GetNativeObject());
    if (!ns_menu) {
      return false;
    }

    // Set the application menu
    [NSApp setMainMenu:ns_menu];

    return true;
  }

  void CleanupEventMonitoring() {
    // Clean up macOS-specific event monitoring
    if (lock_file_handle_ != -1) {
      close(lock_file_handle_);
      lock_file_handle_ = -1;
    }

    if (delegate_) {
      [NSApp setDelegate:nil];
      delegate_ = nil;
    }
  }

 private:
  Application* app_;
  NativeApplicationDelegate* delegate_;
  int lock_file_handle_ = -1;
  NSImageView* dock_icon_view_ = nil;
  NSProgressIndicator* dock_progress_ = nil;
};

Application::Application()
    : initialized_(true), running_(false), exit_code_(0), pimpl_(std::make_unique<Impl>(this)) {
  // Perform platform-specific initialization automatically
  pimpl_->Initialize();

  // Emit application started event
  Emit<ApplicationStartedEvent>();
}

Application::~Application() {
  // Clean up platform-specific event monitoring
  pimpl_->CleanupEventMonitoring();
}

int Application::Run() {
  running_ = true;

  // Start the platform-specific main event loop
  int result = pimpl_->Run();

  running_ = false;

  // Emit exit event
  Emit<ApplicationExitingEvent>(result);

  return result;
}

int Application::Run(std::shared_ptr<Window> window) {
  if (!window) {
    return -1;  // Invalid window
  }

  running_ = true;

  // Start the platform-specific main event loop with window
  int result = pimpl_->Run(window);

  running_ = false;

  // Emit exit event
  Emit<ApplicationExitingEvent>(result);

  return result;
}

void Application::Quit(int exit_code) {
  exit_code_ = exit_code;

  // Emit quit requested event
  Emit<ApplicationQuitRequestedEvent>();

  // Request platform-specific quit
  pimpl_->Quit(exit_code);
}

bool Application::IsRunning() const {
  return running_;
}

bool Application::IsSingleInstance() const {
  return false;
}

bool Application::SetIcon(const std::string& icon_path) {
  return pimpl_->SetIcon(icon_path);
}

bool Application::SetDockIconVisible(bool visible) {
  return pimpl_->SetDockIconVisible(visible);
}

bool Application::SetProgressBar(double progress) {
  return pimpl_->SetProgressBar(progress);
}

bool Application::SetBadgeLabel(const std::string& label) {
  return pimpl_->SetBadgeLabel(label);
}

bool Application::SetBrightness(Brightness brightness) {
  return pimpl_->SetBrightness(brightness);
}

bool Application::SetMenuBar(std::shared_ptr<Menu> menu) {
  return pimpl_->SetMenuBar(menu);
}

std::shared_ptr<Window> Application::GetPrimaryWindow() const {
  return primary_window_;
}

void Application::SetPrimaryWindow(std::shared_ptr<Window> window) {
  primary_window_ = window;
}

std::vector<std::shared_ptr<Window>> Application::GetAllWindows() const {
  auto& window_manager = WindowManager::GetInstance();
  return window_manager.GetAll();
}

}  // namespace nativeapi
