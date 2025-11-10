#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nativeapi.h"

void on_application_event(const native_application_event_t* event) {
  switch (event->type) {
    case NATIVE_APPLICATION_EVENT_STARTED:
      printf("Application started event received\n");
      break;
    case NATIVE_APPLICATION_EVENT_EXITING:
      printf("Application exiting event received with exit code: %d\n", event->exit_code);
      break;
    case NATIVE_APPLICATION_EVENT_ACTIVATED:
      printf("Application activated event received\n");
      break;
    case NATIVE_APPLICATION_EVENT_DEACTIVATED:
      printf("Application deactivated event received\n");
      break;
    case NATIVE_APPLICATION_EVENT_QUIT_REQUESTED:
      printf("Application quit requested event received\n");
      break;
    default:
      printf("Unknown application event type: %d\n", event->type);
      break;
  }
}

int main() {
  printf("Application C API Example\n");

  // Get the Application singleton
  native_application_t app = native_application_get_instance();
  if (!app) {
    fprintf(stderr, "Failed to get application instance\n");
    return 1;
  }

  printf("Application instance obtained successfully\n");
  printf("Single instance: %s\n", native_application_is_single_instance(app) ? "Yes" : "No");

  // Add event listener
  size_t listener_id = native_application_add_event_listener(app, on_application_event);
  if (listener_id == 0) {
    fprintf(stderr, "Failed to add event listener\n");
    return 1;
  }

  // Create a simple window with default settings
  native_window_t window = native_window_manager_create();
  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    return 1;
  }

  // Configure the window
  native_window_set_title(window, "Application C Example Window");
  native_window_set_size(window, 400.0, 300.0, false);

  printf("Window created successfully\n");
  printf("Window ID: %ld\n", native_window_get_id(window));

  // Show the window
  native_window_show(window);

  printf("Starting application event loop...\n");
  printf("Press Ctrl+C to quit\n");

  // Run the application
  int exit_code = native_application_run(app);

  printf("Application exited with code: %d\n", exit_code);

  // Clean up
  native_application_remove_event_listener(app, listener_id);

  return exit_code;
}
