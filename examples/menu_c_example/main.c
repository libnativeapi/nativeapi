#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/capi/application_c.h"
#include "../../src/capi/menu_c.h"
#include "../../src/capi/positioning_strategy_c.h"

// Event callback functions
void on_menu_item_clicked(const void* event, void* user_data) {
  const native_menu_item_clicked_event_t* clicked_event =
      (const native_menu_item_clicked_event_t*)event;
  const char* item_name = (const char*)user_data;

  printf("[EVENT] Menu item clicked: %s (ID: %ld)\n", item_name, clicked_event->item_id);
}

void on_menu_item_submenu_opened(const void* event, void* user_data) {
  const native_menu_item_submenu_opened_event_t* submenu_event =
      (const native_menu_item_submenu_opened_event_t*)event;
  const char* item_name = (const char*)user_data;

  printf("[EVENT] Menu item submenu opened: %s (ID: %ld)\n", item_name, submenu_event->item_id);
}

void on_menu_item_submenu_closed(const void* event, void* user_data) {
  const native_menu_item_submenu_closed_event_t* submenu_event =
      (const native_menu_item_submenu_closed_event_t*)event;
  const char* item_name = (const char*)user_data;

  printf("[EVENT] Menu item submenu closed: %s (ID: %ld)\n", item_name, submenu_event->item_id);
}

void on_menu_opened(const void* event, void* user_data) {
  const native_menu_opened_event_t* open_event = (const native_menu_opened_event_t*)event;
  const char* menu_name = (const char*)user_data;

  printf("[EVENT] Menu opened: %s (ID: %ld)\n", menu_name, open_event->menu_id);
}

void on_menu_closed(const void* event, void* user_data) {
  const native_menu_closed_event_t* close_event = (const native_menu_closed_event_t*)event;
  const char* menu_name = (const char*)user_data;

  printf("[EVENT] Menu closed: %s (ID: %ld)\n", menu_name, close_event->menu_id);
}

int main() {
  printf("=== Menu C API Event System Example ===\n");

  // Create a menu
  native_menu_t menu = native_menu_create();
  if (!menu) {
    printf("Failed to create menu\n");
    return 1;
  }

  printf("Created menu successfully\n");

  // Create menu items
  native_menu_item_t file_item = native_menu_item_create("New File", NATIVE_MENU_ITEM_TYPE_NORMAL);
  native_menu_item_t checkbox_item =
      native_menu_item_create("Word Wrap", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
  native_menu_item_t radio_item1 =
      native_menu_item_create("View Mode 1", NATIVE_MENU_ITEM_TYPE_RADIO);
  native_menu_item_t radio_item2 =
      native_menu_item_create("View Mode 2", NATIVE_MENU_ITEM_TYPE_RADIO);
  native_menu_item_t exit_item = native_menu_item_create("Exit", NATIVE_MENU_ITEM_TYPE_NORMAL);

  if (!file_item || !checkbox_item || !radio_item1 || !radio_item2 || !exit_item) {
    printf("Failed to create menu items\n");
    native_menu_destroy(menu);
    return 1;
  }

  // Set up radio group
  native_menu_item_set_radio_group(radio_item1, 1);
  native_menu_item_set_radio_group(radio_item2, 1);
  native_menu_item_set_state(radio_item1, NATIVE_MENU_ITEM_STATE_CHECKED);

  // Set keyboard accelerators
  native_keyboard_accelerator_t ctrl_n = {NATIVE_ACCELERATOR_MODIFIER_CTRL, "N"};
  native_keyboard_accelerator_t ctrl_q = {NATIVE_ACCELERATOR_MODIFIER_CTRL, "Q"};
  native_menu_item_set_accelerator(file_item, &ctrl_n);
  native_menu_item_set_accelerator(exit_item, &ctrl_q);

  printf("Setting up event listeners using new event system...\n");

  // Add event listeners using the new event system
  int file_listener = native_menu_item_add_listener(file_item, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                                    on_menu_item_clicked, (void*)"New File");

  int checkbox_select_listener = native_menu_item_add_listener(
      checkbox_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_menu_item_clicked, (void*)"Word Wrap");

  int exit_listener = native_menu_item_add_listener(exit_item, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                                    on_menu_item_clicked, (void*)"Exit");

  // Add menu event listeners
  int menu_open_listener =
      native_menu_add_listener(menu, NATIVE_MENU_EVENT_OPENED, on_menu_opened, (void*)"Main Menu");

  int menu_close_listener =
      native_menu_add_listener(menu, NATIVE_MENU_EVENT_CLOSED, on_menu_closed, (void*)"Main Menu");

  // Check if listeners were added successfully
  if (file_listener == -1 || checkbox_select_listener == -1 || exit_listener == -1 ||
      menu_open_listener == -1 || menu_close_listener == -1) {
    printf("Failed to add some event listeners\n");
  } else {
    printf("All event listeners added successfully\n");
    printf(
        "Listener IDs: file=%d, checkbox_select=%d, exit=%d, menu_open=%d, "
        "menu_close=%d\n",
        file_listener, checkbox_select_listener, exit_listener, menu_open_listener,
        menu_close_listener);
  }

  // Add items to menu
  native_menu_add_item(menu, file_item);
  native_menu_add_separator(menu);
  native_menu_add_item(menu, checkbox_item);
  native_menu_add_separator(menu);
  native_menu_add_item(menu, radio_item1);
  native_menu_add_item(menu, radio_item2);
  native_menu_add_separator(menu);
  native_menu_add_item(menu, exit_item);

  printf("Menu created with %zu items\n", native_menu_get_item_count(menu));

  // Add submenu to demonstrate submenu events
  native_menu_t submenu = native_menu_create();
  native_menu_item_t submenu_item1 =
      native_menu_item_create("Submenu Item 1", NATIVE_MENU_ITEM_TYPE_NORMAL);
  native_menu_item_t submenu_item2 =
      native_menu_item_create("Submenu Item 2", NATIVE_MENU_ITEM_TYPE_NORMAL);

  native_menu_add_item(submenu, submenu_item1);
  native_menu_add_item(submenu, submenu_item2);

  native_menu_item_t submenu_parent =
      native_menu_item_create("Tools", NATIVE_MENU_ITEM_TYPE_SUBMENU);
  native_menu_item_set_submenu(submenu_parent, submenu);
  native_menu_add_item(menu, submenu_parent);

  // Add submenu event listeners
  int submenu_open_listener =
      native_menu_item_add_listener(submenu_parent, NATIVE_MENU_ITEM_EVENT_SUBMENU_OPENED,
                                    on_menu_item_submenu_opened, (void*)"Tools");
  int submenu_close_listener =
      native_menu_item_add_listener(submenu_parent, NATIVE_MENU_ITEM_EVENT_SUBMENU_CLOSED,
                                    on_menu_item_submenu_closed, (void*)"Tools");

  printf("Added submenu with %zu items\n", native_menu_get_item_count(submenu));

  // Note: Programmatic event triggering is no longer available via trigger API.
  // Events can only be triggered through actual user interaction.
  printf("\n=== Programmatic Event Triggering Removed ===\n");
  printf(
      "Note: The trigger API has been removed. Events are now only "
      "triggered through user interaction.\n");

  // Demonstrate listener removal
  printf("\n=== Testing Listener Removal ===\n");

  printf("Removing checkbox click listener...\n");
  if (native_menu_item_remove_listener(checkbox_item, checkbox_select_listener)) {
    printf("Checkbox click listener removed successfully\n");
  } else {
    printf("Failed to remove checkbox click listener\n");
  }

  printf(
      "Checkbox item listener removed. Events will now only be triggered "
      "through user interaction.\n");

  // Open menu as context menu (this may not work in console applications)
  printf("\n=== Attempting to Open Context Menu ===\n");
  printf("Note: Context menu display may not work in console applications\n");

  native_point_t point = {100, 100};
  native_positioning_strategy_t strategy = native_positioning_strategy_absolute(&point);
  if (native_menu_open(menu, strategy, NATIVE_PLACEMENT_BOTTOM_START)) {
    printf("Context menu opened successfully (BOTTOM_START placement)\n");
  } else {
    printf("Failed to open context menu (expected in console app)\n");
  }
  native_positioning_strategy_free(strategy);

  // Test additional functionality
  printf("\n=== Testing Additional Functionality ===\n");

  native_menu_item_t additional_item =
      native_menu_item_create("Additional Test", NATIVE_MENU_ITEM_TYPE_NORMAL);

  // Test that we can add multiple listeners for the same event
  int additional_listener1 =
      native_menu_item_add_listener(additional_item, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                    on_menu_item_clicked, (void*)"Additional Test 1");
  int additional_listener2 =
      native_menu_item_add_listener(additional_item, NATIVE_MENU_ITEM_EVENT_CLICKED,
                                    on_menu_item_clicked, (void*)"Additional Test 2");

  printf("Added multiple listeners for the same event\n");
  printf("Multiple listeners can be registered for the same event type.\n");

  // Remove one listener
  native_menu_item_remove_listener(additional_item, additional_listener1);
  printf("Removed first listener. Remaining listener will receive events.\n");

  native_menu_item_destroy(additional_item);

  printf("\n=== Event System Demo Complete ===\n");
  printf("This example demonstrates:\n");
  printf("1. Creating menus and menu items with different types\n");
  printf(
      "2. Using the new event listener API with "
      "native_menu_item_add_listener()\n");
  printf("3. Handling NATIVE_MENU_ITEM_EVENT_CLICKED\n");
  printf("4. Handling NATIVE_MENU_EVENT_OPENED and NATIVE_MENU_EVENT_CLOSED\n");
  printf(
      "5. Handling NATIVE_MENU_ITEM_EVENT_SUBMENU_OPENED and "
      "NATIVE_MENU_ITEM_EVENT_SUBMENU_CLOSED\n");
  printf("6. Event listener removal with native_menu_item_remove_listener()\n");
  printf("7. Multiple listeners for the same event type\n");
  printf("8. Manual state management for checkbox and radio items\n");
  printf("9. Submenu support with event handling\n");

  // Cleanup
  native_menu_item_destroy(file_item);
  native_menu_item_destroy(checkbox_item);
  native_menu_item_destroy(radio_item1);
  native_menu_item_destroy(radio_item2);
  native_menu_item_destroy(exit_item);
  native_menu_item_destroy(submenu_item1);
  native_menu_item_destroy(submenu_item2);
  native_menu_item_destroy(submenu_parent);
  native_menu_destroy(submenu);
  native_menu_destroy(menu);

  return 0;
}
