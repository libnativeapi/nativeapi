#include <nativeapi.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  printf("AutoStart C API Example\n");
  printf("=======================\n\n");

  // Check if auto-start is supported on this platform
  if (!native_autostart_is_supported()) {
    printf("AutoStart is not supported on this platform.\n");
    return 0;
  }

  printf("AutoStart is supported on this platform.\n\n");

  // Create an AutoStart manager with a custom identifier and display name
  native_autostart_t autostart =
      native_autostart_create_with_id_and_name("com.example.myapp", "MyApp");
  if (!autostart) {
    printf("Failed to create AutoStart instance.\n");
    return 1;
  }

  // Print info
  char* id = native_autostart_get_id(autostart);
  char* display_name = native_autostart_get_display_name(autostart);
  char* executable = native_autostart_get_executable_path(autostart);

  printf("ID:           %s\n", id ? id : "(null)");
  printf("Display name: %s\n", display_name ? display_name : "(null)");
  printf("Executable:   %s\n", executable ? executable : "(null)");

  char** arguments = NULL;
  size_t arg_count = 0;
  printf("Arguments:    ");
  if (native_autostart_get_arguments(autostart, &arguments, &arg_count) && arg_count > 0) {
    for (size_t i = 0; i < arg_count; ++i) {
      if (i > 0)
        printf(", ");
      printf("%s", arguments[i] ? arguments[i] : "");
    }
    native_autostart_free_arguments(arguments, arg_count);
  } else {
    printf("(none)");
  }
  printf("\n\n");

  free_c_str(id);
  free_c_str(display_name);
  free_c_str(executable);

  // Check current state
  printf("Currently enabled: %s\n", native_autostart_is_enabled(autostart) ? "yes" : "no");

  // Enable auto-start
  printf("\nEnabling auto-start...\n");
  if (native_autostart_enable(autostart)) {
    printf("  -> Success\n");
    printf("  Is enabled now: %s\n", native_autostart_is_enabled(autostart) ? "yes" : "no");
  } else {
    printf("  -> Failed to enable auto-start\n");
  }

  // Disable auto-start
  printf("\nDisabling auto-start...\n");
  if (native_autostart_disable(autostart)) {
    printf("  -> Success\n");
    printf("  Is enabled now: %s\n", native_autostart_is_enabled(autostart) ? "yes" : "no");
  } else {
    printf("  -> Failed to disable auto-start\n");
  }

  // Demo: customize program and arguments
  printf("\nSetting custom arguments...\n");
  char* current_exe = native_autostart_get_executable_path(autostart);
  const char* custom_args[] = {"--minimized", "--silent"};
  native_autostart_set_program(autostart, current_exe ? current_exe : "", custom_args, 2);
  free_c_str(current_exe);

  char* new_exe = native_autostart_get_executable_path(autostart);
  printf("  Executable: %s\n", new_exe ? new_exe : "(null)");
  free_c_str(new_exe);

  char** new_arguments = NULL;
  size_t new_arg_count = 0;
  printf("  Arguments:  ");
  if (native_autostart_get_arguments(autostart, &new_arguments, &new_arg_count) &&
      new_arg_count > 0) {
    for (size_t i = 0; i < new_arg_count; ++i) {
      if (i > 0)
        printf(", ");
      printf("%s", new_arguments[i] ? new_arguments[i] : "");
    }
    native_autostart_free_arguments(new_arguments, new_arg_count);
  } else {
    printf("(none)");
  }
  printf("\n");

  // Clean up
  native_autostart_destroy(autostart);

  printf("\nDone!\n\n");
  printf("Platform notes:\n");
  printf("  - Windows: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\n");
  printf("  - macOS:   ~/Library/LaunchAgents/<id>.plist\n");
  printf("  - Linux:   ~/.config/autostart/<id>.desktop\n");

  return 0;
}
