#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/capi/menu_c.h"
#include "../../src/capi/app_runner_c.h"

// Event callback functions
void on_menu_item_selected(const void* event, void* user_data) {
    const native_menu_item_selected_event_t* selected_event = (const native_menu_item_selected_event_t*)event;
    const char* item_name = (const char*)user_data;
    
    printf("[EVENT] Menu item selected: %s (ID: %ld, Text: %s)\n", 
           item_name, selected_event->item_id, selected_event->item_text);
}

void on_menu_item_state_changed(const void* event, void* user_data) {
    const native_menu_item_state_changed_event_t* state_event = (const native_menu_item_state_changed_event_t*)event;
    const char* item_name = (const char*)user_data;
    
    printf("[EVENT] Menu item state changed: %s (ID: %ld, Checked: %s)\n", 
           item_name, state_event->item_id, state_event->checked ? "true" : "false");
}

void on_menu_will_open(const void* event, void* user_data) {
    const native_menu_will_open_event_t* open_event = (const native_menu_will_open_event_t*)event;
    const char* menu_name = (const char*)user_data;
    
    printf("[EVENT] Menu will open: %s (ID: %ld)\n", menu_name, open_event->menu_id);
}

void on_menu_will_close(const void* event, void* user_data) {
    const native_menu_will_close_event_t* close_event = (const native_menu_will_close_event_t*)event;
    const char* menu_name = (const char*)user_data;
    
    printf("[EVENT] Menu will close: %s (ID: %ld)\n", menu_name, close_event->menu_id);
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
    native_menu_item_t checkbox_item = native_menu_item_create("Word Wrap", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
    native_menu_item_t radio_item1 = native_menu_item_create("View Mode 1", NATIVE_MENU_ITEM_TYPE_RADIO);
    native_menu_item_t radio_item2 = native_menu_item_create("View Mode 2", NATIVE_MENU_ITEM_TYPE_RADIO);
    native_menu_item_t exit_item = native_menu_item_create("Exit", NATIVE_MENU_ITEM_TYPE_NORMAL);
    
    if (!file_item || !checkbox_item || !radio_item1 || !radio_item2 || !exit_item) {
        printf("Failed to create menu items\n");
        native_menu_destroy(menu);
        return 1;
    }
    
    // Set up radio group
    native_menu_item_set_radio_group(radio_item1, 1);
    native_menu_item_set_radio_group(radio_item2, 1);
    native_menu_item_set_checked(radio_item1, true);
    
    // Set keyboard accelerators
    native_keyboard_accelerator_t ctrl_n = { NATIVE_ACCELERATOR_MODIFIER_CTRL, "N" };
    native_keyboard_accelerator_t ctrl_q = { NATIVE_ACCELERATOR_MODIFIER_CTRL, "Q" };
    native_menu_item_set_accelerator(file_item, &ctrl_n);
    native_menu_item_set_accelerator(exit_item, &ctrl_q);
    
    printf("Setting up event listeners using new event system...\n");
    
    // Add event listeners using the new event system
    int file_listener = native_menu_item_add_listener(file_item, NATIVE_MENU_ITEM_EVENT_SELECTED, 
                                                      on_menu_item_selected, (void*)"New File");
    
    int checkbox_select_listener = native_menu_item_add_listener(checkbox_item, NATIVE_MENU_ITEM_EVENT_SELECTED, 
                                                                 on_menu_item_selected, (void*)"Word Wrap");
    
    int checkbox_state_listener = native_menu_item_add_listener(checkbox_item, NATIVE_MENU_ITEM_EVENT_STATE_CHANGED, 
                                                                on_menu_item_state_changed, (void*)"Word Wrap");
    
    int radio1_state_listener = native_menu_item_add_listener(radio_item1, NATIVE_MENU_ITEM_EVENT_STATE_CHANGED, 
                                                              on_menu_item_state_changed, (void*)"View Mode 1");
    
    int radio2_state_listener = native_menu_item_add_listener(radio_item2, NATIVE_MENU_ITEM_EVENT_STATE_CHANGED, 
                                                              on_menu_item_state_changed, (void*)"View Mode 2");
    
    int exit_listener = native_menu_item_add_listener(exit_item, NATIVE_MENU_ITEM_EVENT_SELECTED, 
                                                      on_menu_item_selected, (void*)"Exit");
    
    // Add menu event listeners
    int menu_open_listener = native_menu_add_listener(menu, NATIVE_MENU_EVENT_WILL_OPEN, 
                                                      on_menu_will_open, (void*)"Main Menu");
    
    int menu_close_listener = native_menu_add_listener(menu, NATIVE_MENU_EVENT_WILL_CLOSE, 
                                                       on_menu_will_close, (void*)"Main Menu");
    
    // Check if listeners were added successfully
    if (file_listener == -1 || checkbox_select_listener == -1 || checkbox_state_listener == -1 ||
        radio1_state_listener == -1 || radio2_state_listener == -1 || exit_listener == -1 ||
        menu_open_listener == -1 || menu_close_listener == -1) {
        printf("Failed to add some event listeners\n");
    } else {
        printf("All event listeners added successfully\n");
        printf("Listener IDs: file=%d, checkbox_select=%d, checkbox_state=%d, radio1=%d, radio2=%d, exit=%d, menu_open=%d, menu_close=%d\n",
               file_listener, checkbox_select_listener, checkbox_state_listener, 
               radio1_state_listener, radio2_state_listener, exit_listener,
               menu_open_listener, menu_close_listener);
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
    
    // Demonstrate programmatic triggering
    printf("\n=== Testing Programmatic Event Triggering ===\n");
    
    printf("Triggering file item...\n");
    native_menu_item_trigger(file_item);
    
    printf("Triggering checkbox item...\n");
    native_menu_item_trigger(checkbox_item);
    
    printf("Triggering checkbox item again...\n");
    native_menu_item_trigger(checkbox_item);
    
    printf("Switching radio button...\n");
    native_menu_item_trigger(radio_item2);
    
    printf("Triggering exit item...\n");
    native_menu_item_trigger(exit_item);
    
    // Demonstrate listener removal
    printf("\n=== Testing Listener Removal ===\n");
    
    printf("Removing checkbox state listener...\n");
    if (native_menu_item_remove_listener(checkbox_item, checkbox_state_listener)) {
        printf("Checkbox state listener removed successfully\n");
    } else {
        printf("Failed to remove checkbox state listener\n");
    }
    
    printf("Triggering checkbox item after removing state listener...\n");
    native_menu_item_trigger(checkbox_item);
    
    // Show menu as context menu (this may not work in console applications)
    printf("\n=== Attempting to Show Context Menu ===\n");
    printf("Note: Context menu display may not work in console applications\n");
    
    if (native_menu_show_as_context_menu(menu, 100, 100)) {
        printf("Context menu shown successfully\n");
    } else {
        printf("Failed to show context menu (expected in console app)\n");
    }
    
    // Test additional functionality
    printf("\n=== Testing Additional Functionality ===\n");
    
    native_menu_item_t additional_item = native_menu_item_create("Additional Test", NATIVE_MENU_ITEM_TYPE_NORMAL);
    
    // Test that we can add multiple listeners for the same event
    int additional_listener1 = native_menu_item_add_listener(additional_item, NATIVE_MENU_ITEM_EVENT_SELECTED, 
                                                             on_menu_item_selected, (void*)"Additional Test 1");
    int additional_listener2 = native_menu_item_add_listener(additional_item, NATIVE_MENU_ITEM_EVENT_SELECTED, 
                                                             on_menu_item_selected, (void*)"Additional Test 2");
    
    printf("Added multiple listeners for the same event\n");
    printf("Triggering item with multiple listeners...\n");
    native_menu_item_trigger(additional_item);
    
    // Remove one listener
    native_menu_item_remove_listener(additional_item, additional_listener1);
    printf("Removed first listener, triggering again...\n");
    native_menu_item_trigger(additional_item);
    
    native_menu_item_destroy(additional_item);
    
    printf("\n=== Event System Demo Complete ===\n");
    printf("This example demonstrates:\n");
    printf("1. Creating menus and menu items with different types\n");
    printf("2. Using the new event listener API with native_menu_item_add_listener()\n");
    printf("3. Handling NATIVE_MENU_ITEM_EVENT_SELECTED and NATIVE_MENU_ITEM_EVENT_STATE_CHANGED\n");
    printf("4. Handling NATIVE_MENU_EVENT_WILL_OPEN and NATIVE_MENU_EVENT_WILL_CLOSE\n");
    printf("5. Programmatic event triggering\n");
    printf("6. Event listener removal with native_menu_item_remove_listener()\n");
    printf("7. Multiple listeners for the same event type\n");
    
    // Cleanup
    native_menu_item_destroy(file_item);
    native_menu_item_destroy(checkbox_item);
    native_menu_item_destroy(radio_item1);
    native_menu_item_destroy(radio_item2);
    native_menu_item_destroy(exit_item);
    native_menu_destroy(menu);
    
    return 0;
}
