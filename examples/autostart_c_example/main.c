#include <nativeapi.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  printf("AutoStart C API Example\n");
  printf("=======================\n\n");

  /* Check if auto-start is supported on this platform */
  if (!native_autostart_is_supported()) {
    printf("AutoStart is not supported on this platform.\n");
    return 0;
  }

  printf("AutoStart is supported on this platform.\n\n");

  /* Create an AutoStart manager with a custom identifier and display name */
  native_autostart_t autostart =
      native_autostart_create_with_id_and_name("com.example.myapp.c", "My C Example App");
  if (!autostart) {
    printf("Failed to create AutoStart instance.\n");
    return 1;
  }

  /* Display current configuration */
  char* id = native_autostart_get_id(autostart);
  char* display_name = native_autostart_get_display_name(autostart);
  char* executable = native_autostart_get_executable_path(autostart);

  printf("AutoStart configuration:\n");
  printf("  ID:           %s\n", id ? id : "(null)");
  printf("  Display name: %s\n", display_name ? display_name : "(null)");
  printf("  Executable:   %s\n\n", executable ? executable : "(null)");

  free_c_str(id);
  free_c_str(display_name);

  /* Set a custom program path and arguments */
  const char* arguments[] = {"--minimized", "--autostart"};
  native_autostart_set_program(autostart, executable ? executable : "", arguments, 2);
  free_c_str(executable);

  /* Retrieve and display the updated executable path */
  char* exec_path = native_autostart_get_executable_path(autostart);
  printf("After SetProgram:\n");
  printf("  Executable: %s\n", exec_path ? exec_path : "(null)");
  printf("  Arguments:  --minimized --autostart\n\n");
  free_c_str(exec_path);

  /* Check current state before enabling */
  printf("Is enabled (before Enable): %s\n",
         native_autostart_is_enabled(autostart) ? "yes" : "no");

  /* Enable auto-start */
  printf("Enabling auto-start...\n");
  if (native_autostart_enable(autostart)) {
    printf("Auto-start enabled successfully.\n");
  } else {
    printf("Failed to enable auto-start.\n");
    native_autostart_destroy(autostart);
    return 1;
  }

  /* Verify it is now enabled */
  printf("Is enabled (after Enable):  %s\n\n",
         native_autostart_is_enabled(autostart) ? "yes" : "no");

  /* Update the display name and re-enable to update the stored entry */
  native_autostart_set_display_name(autostart, "My C Example App (Updated)");
  char* updated_name = native_autostart_get_display_name(autostart);
  printf("Updated display name to: %s\n", updated_name ? updated_name : "(null)");
  free_c_str(updated_name);
  native_autostart_enable(autostart);

  /* Disable auto-start */
  printf("\nDisabling auto-start...\n");
  if (native_autostart_disable(autostart)) {
    printf("Auto-start disabled successfully.\n");
  } else {
    printf("Failed to disable auto-start.\n");
    native_autostart_destroy(autostart);
    return 1;
  }

  /* Verify it is now disabled */
  printf("Is enabled (after Disable): %s\n\n",
         native_autostart_is_enabled(autostart) ? "yes" : "no");

  /* Clean up */
  native_autostart_destroy(autostart);

  printf("Example completed successfully!\n\n");
  printf("This example demonstrated:\n");
  printf("  * native_autostart_is_supported()              - Check platform support\n");
  printf("  * native_autostart_create_with_id_and_name()   - Create with ID and name\n");
  printf("  * native_autostart_get_id()                    - Get identifier\n");
  printf("  * native_autostart_get_display_name()          - Get display name\n");
  printf("  * native_autostart_set_display_name()          - Update display name\n");
  printf("  * native_autostart_set_program()               - Set executable and arguments\n");
  printf("  * native_autostart_get_executable_path()       - Get configured executable\n");
  printf("  * native_autostart_enable()                    - Register auto-start with the OS\n");
  printf("  * native_autostart_disable()                   - Remove auto-start from the OS\n");
  printf("  * native_autostart_is_enabled()                - Query current registration state\n");
  printf("  * native_autostart_destroy()                   - Free resources\n");

  return 0;
}
