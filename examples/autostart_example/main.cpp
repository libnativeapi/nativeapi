#include <nativeapi.h>
#include <iostream>

using namespace nativeapi;

int main() {
  std::cout << "AutoStart Example" << std::endl;
  std::cout << "=================" << std::endl;
  std::cout << std::endl;

  // Check if auto-start is supported on this platform
  if (!AutoStart::IsSupported()) {
    std::cout << "AutoStart is not supported on this platform." << std::endl;
    return 0;
  }

  std::cout << "AutoStart is supported on this platform." << std::endl;
  std::cout << std::endl;

  // Create an AutoStart manager with a custom identifier and display name
  AutoStart autostart("com.example.myapp", "MyApp");

  std::cout << "ID:           " << autostart.GetId() << std::endl;
  std::cout << "Display name: " << autostart.GetDisplayName() << std::endl;
  std::cout << "Executable:   " << autostart.GetExecutablePath() << std::endl;

  auto args = autostart.GetArguments();
  std::cout << "Arguments:    ";
  if (args.empty()) {
    std::cout << "(none)";
  } else {
    for (size_t i = 0; i < args.size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << args[i];
    }
  }
  std::cout << std::endl;
  std::cout << std::endl;

  // Check current state
  std::cout << "Currently enabled: " << (autostart.IsEnabled() ? "yes" : "no") << std::endl;

  // Enable auto-start
  std::cout << std::endl;
  std::cout << "Enabling auto-start..." << std::endl;
  if (autostart.Enable()) {
    std::cout << "  -> Success" << std::endl;
    std::cout << "  Is enabled now: " << (autostart.IsEnabled() ? "yes" : "no") << std::endl;
  } else {
    std::cout << "  -> Failed to enable auto-start" << std::endl;
  }

  // Disable auto-start
  std::cout << std::endl;
  std::cout << "Disabling auto-start..." << std::endl;
  if (autostart.Disable()) {
    std::cout << "  -> Success" << std::endl;
    std::cout << "  Is enabled now: " << (autostart.IsEnabled() ? "yes" : "no") << std::endl;
  } else {
    std::cout << "  -> Failed to disable auto-start" << std::endl;
  }

  // Demo: customize the program and arguments
  std::cout << std::endl;
  std::cout << "Setting custom program path and arguments..." << std::endl;
  autostart.SetProgram(autostart.GetExecutablePath(), {"--minimized", "--silent"});
  std::cout << "  Executable: " << autostart.GetExecutablePath() << std::endl;

  auto new_args = autostart.GetArguments();
  std::cout << "  Arguments:  ";
  for (size_t i = 0; i < new_args.size(); ++i) {
    if (i > 0)
      std::cout << ", ";
    std::cout << new_args[i];
  }
  std::cout << std::endl;

  std::cout << std::endl;
  std::cout << "Done!" << std::endl;
  std::cout << std::endl;
  std::cout << "Platform notes:" << std::endl;
  std::cout << "  - Windows: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" << std::endl;
  std::cout << "  - macOS:   ~/Library/LaunchAgents/<id>.plist" << std::endl;
  std::cout << "  - Linux:   ~/.config/autostart/<id>.desktop" << std::endl;

  return 0;
}
