#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Include individual C API headers instead of the full nativeapi.h
#include "../../src/capi/menu_c.h"
#include "../../src/capi/run_example_app_c.h"
#include "../../src/capi/string_utils_c.h"
#include "../../src/capi/tray_icon_c.h"
#include "../../src/capi/tray_manager_c.h"

// Event callback functions
void on_menu_item_clicked(const void* event, void* user_data) {
  const native_menu_item_clicked_event_t* clicked_event =
      (const native_menu_item_clicked_event_t*)event;
  printf("Menu item clicked: ID=%ld, Text='%s'\n", clicked_event->item_id,
         clicked_event->item_text);

  if (strcmp(clicked_event->item_text, "Exit") == 0) {
    printf("Exiting application...\n");
    exit(0);
  } else if (strcmp(clicked_event->item_text, "Show Message") == 0) {
    printf("Hello from tray menu!\n");
  }
}

void on_tray_clicked(const void* event, void* user_data) {
  const native_tray_icon_clicked_event_t* clicked_event =
      (const native_tray_icon_clicked_event_t*)event;
  printf("Tray icon clicked! ID=%ld\n", clicked_event->tray_icon_id);
}

void on_tray_right_clicked(const void* event, void* user_data) {
  const native_tray_icon_right_clicked_event_t* right_clicked_event =
      (const native_tray_icon_right_clicked_event_t*)event;
  printf("Tray icon right clicked! ID=%ld\n",
         right_clicked_event->tray_icon_id);
}

void on_tray_double_clicked(const void* event, void* user_data) {
  const native_tray_icon_double_clicked_event_t* double_clicked_event =
      (const native_tray_icon_double_clicked_event_t*)event;
  printf("Tray icon double clicked! ID=%ld\n",
         double_clicked_event->tray_icon_id);
}

void on_menu_opened(const void* event, void* user_data) {
  const native_menu_opened_event_t* open_event =
      (const native_menu_opened_event_t*)event;
  printf("Menu opened: ID=%ld\n", open_event->menu_id);
}

void on_menu_closed(const void* event, void* user_data) {
  const native_menu_closed_event_t* close_event =
      (const native_menu_closed_event_t*)event;
  printf("Menu closed: ID=%ld\n", close_event->menu_id);
}

int main() {
  printf("=== Tray Menu C API Example ===\n");

  // Check if system tray is supported
  if (!native_tray_manager_is_supported()) {
    printf("Error: System tray is not supported on this platform!\n");
    return 1;
  }

  printf("System tray is supported.\n");

  // Create a menu
  native_menu_t menu = native_menu_create();
  if (!menu) {
    printf("Error: Failed to create menu!\n");
    return 1;
  }

  printf("Created menu with ID: %ld\n", native_menu_get_id(menu));

  // Create menu items
  native_menu_item_t item1 =
      native_menu_item_create("Show Message", NATIVE_MENU_ITEM_TYPE_NORMAL);
  native_menu_item_t item2 =
      native_menu_item_create("Settings", NATIVE_MENU_ITEM_TYPE_NORMAL);
  native_menu_item_t checkbox = native_menu_item_create(
      "Enable Notifications", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
  native_menu_item_t separator = native_menu_item_create_separator();
  native_menu_item_t exit_item =
      native_menu_item_create("Exit", NATIVE_MENU_ITEM_TYPE_NORMAL);

  if (!item1 || !item2 || !checkbox || !separator || !exit_item) {
    printf("Error: Failed to create menu items!\n");
    native_menu_destroy(menu);
    return 1;
  }

  // Set up menu item properties
  native_menu_item_set_enabled(item1, true);
  native_menu_item_set_tooltip(item1, "Click to show a message");

  // Set up keyboard accelerator for exit item
  native_keyboard_accelerator_t exit_accel = {
      .modifiers = NATIVE_ACCELERATOR_MODIFIER_CTRL, .key = "Q"};
  native_menu_item_set_accelerator(exit_item, &exit_accel);

  // Set checkbox state
  native_menu_item_set_state(checkbox, NATIVE_MENU_ITEM_STATE_CHECKED);

  // Set up event listeners using new API
  native_menu_item_add_listener(item1, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                on_menu_item_clicked, NULL);
  native_menu_item_add_listener(item2, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                on_menu_item_clicked, NULL);
  native_menu_item_add_listener(exit_item, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                on_menu_item_clicked, NULL);
  native_menu_item_add_listener(checkbox, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                on_menu_item_clicked, NULL);

  // Add items to menu
  native_menu_add_item(menu, item1);
  native_menu_add_item(menu, item2);
  native_menu_add_item(menu, checkbox);
  native_menu_add_item(menu, separator);
  native_menu_add_item(menu, exit_item);

  printf("Added %zu items to menu\n", native_menu_get_item_count(menu));

  // Set menu event listeners using new API
  native_menu_add_listener(menu, NATIVE_MENU_EVENT_OPENED, on_menu_opened,
                           NULL);
  native_menu_add_listener(menu, NATIVE_MENU_EVENT_CLOSED, on_menu_closed,
                           NULL);

  // Create a submenu example
  native_menu_t submenu = native_menu_create();
  if (submenu) {
    native_menu_item_t sub_item1 =
        native_menu_item_create("Sub Item 1", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_t sub_item2 =
        native_menu_item_create("Sub Item 2", NATIVE_MENU_ITEM_TYPE_NORMAL);

    if (sub_item1 && sub_item2) {
      native_menu_add_item(submenu, sub_item1);
      native_menu_add_item(submenu, sub_item2);

      // Create submenu item and add to main menu
      native_menu_item_t submenu_item = native_menu_item_create(
          "More Options", NATIVE_MENU_ITEM_TYPE_SUBMENU);
      if (submenu_item) {
        native_menu_item_set_submenu(submenu_item, submenu);
        native_menu_add_item(menu, submenu_item);
        printf("Created submenu with %zu items\n",
               native_menu_get_item_count(submenu));
      }
    }
  }

  // Create tray icon
  native_tray_icon_t tray_icon = native_tray_icon_create();
  if (!tray_icon) {
    printf("Error: Failed to create tray icon!\n");
    native_menu_destroy(menu);
    return 1;
  }

  printf("Created tray icon with ID: %ld\n",
         native_tray_icon_get_id(tray_icon));

  // Set up tray icon properties
  native_tray_icon_set_title(tray_icon, "My App");
  native_tray_icon_set_tooltip(tray_icon,
                               "My Application - Right click for menu");

  // Set the context menu
  native_tray_icon_set_context_menu(tray_icon, menu);

  // Set up tray icon event listeners using new API
  native_tray_icon_add_listener(tray_icon, NATIVE_TRAY_ICON_EVENT_CLICKED,
                                on_tray_clicked, NULL);
  native_tray_icon_add_listener(tray_icon, NATIVE_TRAY_ICON_EVENT_RIGHT_CLICKED,
                                on_tray_right_clicked, NULL);
  native_tray_icon_add_listener(tray_icon,
                                NATIVE_TRAY_ICON_EVENT_DOUBLE_CLICKED,
                                on_tray_double_clicked, NULL);

  // Show the tray icon
  if (native_tray_icon_set_visible(tray_icon, true)) {
    printf("Tray icon is now visible\n");
  } else {
    printf("Warning: Failed to show tray icon\n");
  }

  // Get tray icon bounds
  native_rectangle_t bounds;
  if (native_tray_icon_get_bounds(tray_icon, &bounds)) {
    printf("Tray icon bounds: x=%.1f, y=%.1f, width=%.1f, height=%.1f\n",
           bounds.x, bounds.y, bounds.width, bounds.height);
  }

  // Show all managed tray icons
  native_tray_icon_list_t tray_list = native_tray_manager_get_all();
  printf("Total managed tray icons: %zu\n", tray_list.count);
  native_tray_icon_list_free(tray_list);

  printf("\n=== Tray icon and menu are now active ===\n");
  printf("- Click the tray icon to see click message\n");
  printf("- Right click the tray icon to open context menu\n");
  printf("- Double click the tray icon to see double click message\n");
  printf("- Use menu items to interact with the application\n");
  printf("- Click 'Exit' to quit\n");
  printf("\nRunning... (Press Ctrl+C to force quit)\n");

  int exit_code = native_run_example_app();

  return 0;
}
