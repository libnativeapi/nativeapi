#include <iostream>
#include <string>
#include <vector>

#include "nativeapi.h"

using namespace nativeapi;

int main() {
  std::cout << "AutoStart Example\n";
  std::cout << "=================\n\n";

  // Check if auto-start is supported on this platform
  if (!AutoStart::IsSupported()) {
    std::cout << "AutoStart is not supported on this platform.\n";
    return 0;
  }

  std::cout << "AutoStart is supported on this platform.\n\n";

  // Create an AutoStart manager with a custom identifier and display name
  AutoStart autostart("com.example.myapp", "My Example App");

  // Display current configuration
  std::cout << "AutoStart configuration:\n";
  std::cout << "  ID:           " << autostart.GetId() << "\n";
  std::cout << "  Display name: " << autostart.GetDisplayName() << "\n";
  std::cout << "  Executable:   " << autostart.GetExecutablePath() << "\n\n";

  // Set a custom program path and arguments
  autostart.SetProgram(autostart.GetExecutablePath(), {"--minimized", "--autostart"});

  std::cout << "After SetProgram:\n";
  std::cout << "  Executable: " << autostart.GetExecutablePath() << "\n";
  auto args = autostart.GetArguments();
  std::cout << "  Arguments:  ";
  for (const auto& arg : args) {
    std::cout << arg << " ";
  }
  std::cout << "\n\n";

  // Check current state before enabling
  std::cout << "Is enabled (before Enable): " << (autostart.IsEnabled() ? "yes" : "no") << "\n";

  // Enable auto-start
  std::cout << "Enabling auto-start...\n";
  if (autostart.Enable()) {
    std::cout << "Auto-start enabled successfully.\n";
  } else {
    std::cout << "Failed to enable auto-start.\n";
    return 1;
  }

  // Verify it is now enabled
  std::cout << "Is enabled (after Enable):  " << (autostart.IsEnabled() ? "yes" : "no") << "\n\n";

  // Update the display name and re-enable to update the stored entry
  autostart.SetDisplayName("My Example App (Updated)");
  std::cout << "Updated display name to: " << autostart.GetDisplayName() << "\n";
  autostart.Enable();

  // Disable auto-start
  std::cout << "\nDisabling auto-start...\n";
  if (autostart.Disable()) {
    std::cout << "Auto-start disabled successfully.\n";
  } else {
    std::cout << "Failed to disable auto-start.\n";
    return 1;
  }

  // Verify it is now disabled
  std::cout << "Is enabled (after Disable): " << (autostart.IsEnabled() ? "yes" : "no") << "\n\n";

  std::cout << "Example completed successfully!\n\n";
  std::cout << "This example demonstrated:\n";
  std::cout << "  * AutoStart::IsSupported()    - Check platform support\n";
  std::cout << "  * AutoStart(id, name)         - Construct with identifier and display name\n";
  std::cout << "  * AutoStart::GetId()          - Get identifier\n";
  std::cout << "  * AutoStart::GetDisplayName() - Get display name\n";
  std::cout << "  * AutoStart::SetDisplayName() - Update display name\n";
  std::cout << "  * AutoStart::SetProgram()     - Set executable path and arguments\n";
  std::cout << "  * AutoStart::GetExecutablePath() - Get configured executable\n";
  std::cout << "  * AutoStart::GetArguments()   - Get configured arguments\n";
  std::cout << "  * AutoStart::Enable()         - Register auto-start with the OS\n";
  std::cout << "  * AutoStart::Disable()        - Remove auto-start from the OS\n";
  std::cout << "  * AutoStart::IsEnabled()      - Query current registration state\n";

  return 0;
}
