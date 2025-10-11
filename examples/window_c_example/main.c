#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include C API headers
#include "../../src/capi/accessibility_manager_c.h"
#include "../../src/capi/tray_manager_c.h"
#include "../../src/capi/tray_icon_c.h"
#include "../../src/capi/window_manager_c.h"
#include "../../src/capi/window_c.h"
#include "../../src/capi/menu_c.h"
#include "../../src/capi/run_example_app_c.h"
#include "../../src/capi/string_utils_c.h"

// Global variables to store handles
static native_window_t g_window = NULL;
static native_tray_icon_t g_tray_icon = NULL;
static native_menu_t g_context_menu = NULL;

// Event callback functions
void on_tray_icon_clicked(const void* event, void* user_data) {
    const native_tray_icon_clicked_event_t* clicked_event = (const native_tray_icon_clicked_event_t*)event;
    printf("*** TRAY ICON LEFT CLICKED! ***\n");
    printf("This is the left click handler working!\n");
    printf("Tray icon ID: %ld\n", clicked_event->tray_icon_id);
}

void on_tray_icon_right_clicked(const void* event, void* user_data) {
    const native_tray_icon_right_clicked_event_t* right_clicked_event = (const native_tray_icon_right_clicked_event_t*)event;
    printf("*** TRAY ICON RIGHT CLICKED! ***\n");
    printf("This is the right click handler working!\n");
    printf("Tray icon ID: %ld\n", right_clicked_event->tray_icon_id);
}

void on_tray_icon_double_clicked(const void* event, void* user_data) {
    const native_tray_icon_double_clicked_event_t* double_clicked_event = (const native_tray_icon_double_clicked_event_t*)event;
    printf("*** TRAY ICON DOUBLE CLICKED! ***\n");
    printf("This is the double click handler working!\n");
    printf("Tray icon ID: %ld\n", double_clicked_event->tray_icon_id);
}

void on_show_window_clicked(const void* event, void* user_data) {
    printf("Show Window clicked from context menu\n");
    if (g_window) {
        native_window_show(g_window);
        native_window_focus(g_window);
    }
}

void on_hide_window_clicked(const void* event, void* user_data) {
    printf("Hide Window clicked from context menu\n");
    if (g_window) {
        native_window_hide(g_window);
    }
}

void on_about_clicked(const void* event, void* user_data) {
    printf("About clicked from context menu\n");
    printf("Window Example v1.0 - Native API Demo\n");
}

void on_clear_cache_clicked(const void* event, void* user_data) {
    printf("Clear Cache clicked from submenu\n");
}

void on_reset_settings_clicked(const void* event, void* user_data) {
    printf("Reset Settings clicked from submenu\n");
}

void on_debug_mode_clicked(const void* event, void* user_data) {
    native_menu_item_t debug_mode_item = (native_menu_item_t)user_data;
    native_menu_item_state_t current_state = native_menu_item_get_state(debug_mode_item);
    native_menu_item_state_t new_state = (current_state == NATIVE_MENU_ITEM_STATE_CHECKED) ?
                                         NATIVE_MENU_ITEM_STATE_UNCHECKED : NATIVE_MENU_ITEM_STATE_CHECKED;
    native_menu_item_set_state(debug_mode_item, new_state);
    printf("Debug Mode %s\n", (new_state == NATIVE_MENU_ITEM_STATE_CHECKED) ? "enabled" : "disabled");
}

void on_auto_start_clicked(const void* event, void* user_data) {
    native_menu_item_t auto_start_item = (native_menu_item_t)user_data;
    native_menu_item_state_t current_state = native_menu_item_get_state(auto_start_item);
    native_menu_item_state_t new_state = (current_state == NATIVE_MENU_ITEM_STATE_CHECKED) ?
                                         NATIVE_MENU_ITEM_STATE_UNCHECKED : NATIVE_MENU_ITEM_STATE_CHECKED;
    native_menu_item_set_state(auto_start_item, new_state);
    printf("Auto Start %s\n", (new_state == NATIVE_MENU_ITEM_STATE_CHECKED) ? "enabled" : "disabled");
}

void on_notifications_clicked(const void* event, void* user_data) {
    native_menu_item_t notifications_item = (native_menu_item_t)user_data;
    native_menu_item_state_t current_state = native_menu_item_get_state(notifications_item);
    native_menu_item_state_t new_state = (current_state == NATIVE_MENU_ITEM_STATE_CHECKED) ?
                                         NATIVE_MENU_ITEM_STATE_UNCHECKED : NATIVE_MENU_ITEM_STATE_CHECKED;
    native_menu_item_set_state(notifications_item, new_state);
    printf("Notifications %s\n", (new_state == NATIVE_MENU_ITEM_STATE_CHECKED) ? "enabled" : "disabled");
}

void on_sync_item_clicked(const void* event, void* user_data) {
    native_menu_item_t sync_item = (native_menu_item_t)user_data;
    native_menu_item_state_t current_state = native_menu_item_get_state(sync_item);
    native_menu_item_state_t next_state;
    const char* state_name;

    // Cycle through states: Mixed -> Checked -> Unchecked -> Mixed
    switch (current_state) {
        case NATIVE_MENU_ITEM_STATE_MIXED:
            next_state = NATIVE_MENU_ITEM_STATE_CHECKED;
            state_name = "enabled";
            break;
        case NATIVE_MENU_ITEM_STATE_CHECKED:
            next_state = NATIVE_MENU_ITEM_STATE_UNCHECKED;
            state_name = "disabled";
            break;
        case NATIVE_MENU_ITEM_STATE_UNCHECKED:
        default:
            next_state = NATIVE_MENU_ITEM_STATE_MIXED;
            state_name = "partial";
            break;
    }

    native_menu_item_set_state(sync_item, next_state);
    printf("Sync Status: %s\n", state_name);
}

void on_light_theme_clicked(const void* event, void* user_data) {
    native_menu_item_t light_theme_item = (native_menu_item_t)user_data;
    native_menu_item_set_state(light_theme_item, NATIVE_MENU_ITEM_STATE_CHECKED);
    printf("Light theme selected\n");
}

void on_dark_theme_clicked(const void* event, void* user_data) {
    native_menu_item_t dark_theme_item = (native_menu_item_t)user_data;
    native_menu_item_set_state(dark_theme_item, NATIVE_MENU_ITEM_STATE_CHECKED);
    printf("Dark theme selected\n");
}

void on_auto_theme_clicked(const void* event, void* user_data) {
    native_menu_item_t auto_theme_item = (native_menu_item_t)user_data;
    native_menu_item_set_state(auto_theme_item, NATIVE_MENU_ITEM_STATE_CHECKED);
    printf("Auto theme selected\n");
}

void on_exit_clicked(const void* event, void* user_data) {
    printf("Exit clicked from context menu\n");
    // Get all windows and destroy them to trigger app exit
    native_window_list_t windows = native_window_manager_get_all();
    for (long i = 0; i < windows.count; i++) {
        native_window_id_t window_id = native_window_get_id(windows.windows[i]);
        native_window_manager_destroy(window_id);
    }
    native_window_list_free(&windows);
}

void on_tools_submenu_opened(const void* event, void* user_data) {
    const native_menu_item_submenu_opened_event_t* opened_event = (const native_menu_item_submenu_opened_event_t*)event;
    printf("Tools submenu opened (ID: %ld)\n", opened_event->item_id);
}

void on_tools_submenu_closed(const void* event, void* user_data) {
    const native_menu_item_submenu_closed_event_t* closed_event = (const native_menu_item_submenu_closed_event_t*)event;
    printf("Tools submenu closed (ID: %ld)\n", closed_event->item_id);
}

native_menu_t create_context_menu(void) {
    // Create context menu
    native_menu_t context_menu = native_menu_create();

    // Add Show Window item
    native_menu_item_t show_window_item = native_menu_item_create("Show Window", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_add_listener(show_window_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_show_window_clicked, NULL);
    native_menu_add_item(context_menu, show_window_item);

    // Add Hide Window item
    native_menu_item_t hide_window_item = native_menu_item_create("Hide Window", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_add_listener(hide_window_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_hide_window_clicked, NULL);
    native_menu_add_item(context_menu, hide_window_item);

    // Add separator
    native_menu_add_separator(context_menu);

    // Add About item
    native_menu_item_t about_item = native_menu_item_create("About", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_add_listener(about_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_about_clicked, NULL);
    native_menu_add_item(context_menu, about_item);

    // Create Tools submenu
    native_menu_t tools_submenu = native_menu_create();

    // Add items to tools submenu
    native_menu_item_t clear_cache_item = native_menu_item_create("Clear Cache", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_add_listener(clear_cache_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_clear_cache_clicked, NULL);
    native_menu_add_item(tools_submenu, clear_cache_item);

    native_menu_item_t reset_settings_item = native_menu_item_create("Reset Settings", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_add_listener(reset_settings_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_reset_settings_clicked, NULL);
    native_menu_add_item(tools_submenu, reset_settings_item);

    native_menu_add_separator(tools_submenu);

    native_menu_item_t debug_mode_item = native_menu_item_create("Debug Mode", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
    native_menu_item_set_state(debug_mode_item, NATIVE_MENU_ITEM_STATE_UNCHECKED);
    native_menu_item_add_listener(debug_mode_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_debug_mode_clicked, debug_mode_item);
    native_menu_add_item(tools_submenu, debug_mode_item);

    // Create the submenu parent item
    native_menu_item_t tools_item = native_menu_item_create("Tools", NATIVE_MENU_ITEM_TYPE_SUBMENU);
    native_menu_item_set_submenu(tools_item, tools_submenu);

    // Add submenu event listeners
    native_menu_item_add_listener(tools_item, NATIVE_MENU_ITEM_EVENT_SUBMENU_OPENED, on_tools_submenu_opened, NULL);
    native_menu_item_add_listener(tools_item, NATIVE_MENU_ITEM_EVENT_SUBMENU_CLOSED, on_tools_submenu_closed, NULL);

    native_menu_add_item(context_menu, tools_item);

    // Add separator before preferences
    native_menu_add_separator(context_menu);

    // Add preferences item
    native_menu_item_t preferences_item = native_menu_item_create("Preferences", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_add_item(context_menu, preferences_item);

    // Add checkbox menu items
    native_menu_item_t auto_start_item = native_menu_item_create("Auto Start", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
    native_menu_item_set_state(auto_start_item, NATIVE_MENU_ITEM_STATE_CHECKED); // Initially checked
    native_menu_item_add_listener(auto_start_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_auto_start_clicked, auto_start_item);
    native_menu_add_item(context_menu, auto_start_item);

    native_menu_item_t notifications_item = native_menu_item_create("Show Notifications", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
    native_menu_item_set_state(notifications_item, NATIVE_MENU_ITEM_STATE_UNCHECKED); // Initially unchecked
    native_menu_item_add_listener(notifications_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_notifications_clicked, notifications_item);
    native_menu_add_item(context_menu, notifications_item);

    // Add three-state checkbox example
    native_menu_item_t sync_item = native_menu_item_create("Sync Status", NATIVE_MENU_ITEM_TYPE_CHECKBOX);
    native_menu_item_set_state(sync_item, NATIVE_MENU_ITEM_STATE_MIXED); // Initially mixed/indeterminate
    native_menu_item_add_listener(sync_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_sync_item_clicked, sync_item);
    native_menu_add_item(context_menu, sync_item);

    // Add separator before radio group
    native_menu_add_separator(context_menu);

    // Add radio button group for theme selection
    native_menu_item_t theme_label = native_menu_item_create("Theme:", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_add_item(context_menu, theme_label);

    native_menu_item_t light_theme_item = native_menu_item_create("Light Theme", NATIVE_MENU_ITEM_TYPE_RADIO);
    native_menu_item_set_radio_group(light_theme_item, 0); // Group 0
    native_menu_item_set_state(light_theme_item, NATIVE_MENU_ITEM_STATE_CHECKED); // Default selection
    native_menu_item_add_listener(light_theme_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_light_theme_clicked, light_theme_item);
    native_menu_add_item(context_menu, light_theme_item);

    native_menu_item_t dark_theme_item = native_menu_item_create("Dark Theme", NATIVE_MENU_ITEM_TYPE_RADIO);
    native_menu_item_set_radio_group(dark_theme_item, 0); // Same group as light theme
    native_menu_item_add_listener(dark_theme_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_dark_theme_clicked, dark_theme_item);
    native_menu_add_item(context_menu, dark_theme_item);

    native_menu_item_t auto_theme_item = native_menu_item_create("Auto Theme", NATIVE_MENU_ITEM_TYPE_RADIO);
    native_menu_item_set_radio_group(auto_theme_item, 0); // Same group
    native_menu_item_add_listener(auto_theme_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_auto_theme_clicked, auto_theme_item);
    native_menu_add_item(context_menu, auto_theme_item);

    // Add another separator
    native_menu_add_separator(context_menu);

    // Add exit item
    native_menu_item_t exit_item = native_menu_item_create("Exit", NATIVE_MENU_ITEM_TYPE_NORMAL);
    native_menu_item_add_listener(exit_item, NATIVE_MENU_ITEM_EVENT_CLICKED, on_exit_clicked, NULL);
    native_menu_add_item(context_menu, exit_item);

    return context_menu;
}

int main() {
    // Enable accessibility
    native_accessibility_manager_enable();
    bool is_enabled = native_accessibility_manager_is_enabled();
    printf("is_enabled: %s\n", is_enabled ? "true" : "false");

    // Initialize managers
    native_window_manager_initialize();

    // Create a new window with options
    native_window_options_t* options = native_window_options_create();
    native_window_options_set_title(options, "Window Example");
    native_window_options_set_size(options, 800, 600);
    native_window_options_set_minimum_size(options, 400, 300);
    native_window_options_set_maximum_size(options, 1920, 1080);
    native_window_options_set_centered(options, true);

    g_window = native_window_manager_create(options);
    native_window_options_destroy(options);

    // Create tray icon
    g_tray_icon = native_tray_manager_create();
    if (g_tray_icon != NULL) {
        native_tray_icon_set_icon(g_tray_icon,
            "data:image/png;base64,"
            "iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAABGdBTUEAALGPC/"
            "xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAhGVY"
            "SWZNTQAqAAAACAAFARIAAwAAAAEAAQAAARoABQAAAAEAAABKARsABQAAAAEAAABSASgAAw"
            "AAAAEAAgAAh2kABAAAAAEAAABaAAAAAAAAAEgAAAABAAAASAAAAAEAA6ABAAMAAAABAAEA"
            "AKACAAQAAAABAAAAFKADAAQAAAABAAAAFAAAAABB553+"
            "AAAACXBIWXMAAAsTAAALEwEAmpwYAAABWWlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPH"
            "g6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUg"
            "Ni4wLjAiPgogICA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OT"
            "kvMDIvMjItcmRmLXN5bnRheC1ucyMiPgogICAgICA8cmRmOkRlc2NyaXB0aW9uIHJkZjph"
            "Ym91dD0iIgogICAgICAgICAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3"
            "RpZmYvMS4wLyI+"
            "CiAgICAgICAgIDx0aWZmOk9yaWVudGF0aW9uPjE8L3RpZmY6T3JpZW50YXRpb24+"
            "CiAgICAgIDwvcmRmOkRlc2NyaXB0aW9uPgogICA8L3JkZjpSREY+"
            "CjwveDp4bXBtZXRhPgoZXuEHAAADOElEQVQ4EY1USUtbURQ+N+/"
            "Fh1O04IxYHFAQNKiIIOJGV10oVbEb3fgHJLpKf0DjQuim3RS3roR25c7qzoWgkCAiiAM4I"
            "W5MnPUlt+c75j6iCdILN+/cM3xnzFFEpPjqYDD4WWsd4htUSgXAS8v4k3VExroJ1o3y/"
            "R6NRv+wlrKg2t7e/s2yrB+2bX8sKChwwHNdl/"
            "Xg6+WwMTmOQ+Alk0mR+Xw+Bzb8+FJRUeFcXFz8VYiMBb/ZzE0kEnp/"
            "f99mWnV2dtLz87MAAMzv99PR0RGVlJRQaWkpQCkvL49F2oV+KpWy+Y5YlZWVv+"
            "Dl6uoq2dra6p+enlYtLS20vr4uhqwkYCcnJzQ0NCQ8dkqFhYW0tbWlzs/"
            "PrcfHx2RZWZnFAdRQW1tbvLe3FzVJzc/Pw6OOxWJ4a/C5HLqvr0/"
            "ex8fHenV1VWjIFxcX9cHBgR4YGEg1Nzfrjo6OuM35BwCCsPnKQTo4SJNrQysrKzQxMUG1t"
            "bVSRxGm5aDZXBrLZMD3FgwK5uzs7AhYXV0dhUIhYZeXl9PS0pLQ4+"
            "Pj1NDQQLu7u1RUVKS4kdrHEi8yA4RO4kxNTdHm5iZtb28TmvSSCNHY2BhdXl7S2toajY6O"
            "UiAQINQarfcZYwOGr+"
            "FVV1dTU1MTFRcXe2IDyk2gxsZG6TqPC3FjRQcRZh14w5mZmRGDcDhMp6en4gjOuLs0OTlJ"
            "KMXy8jLV19cTd1psXmCFzP7p7+"
            "WVObm5kQYiUTo+vqahoeHCWPE3ae7uztvXqGUM0IDDa83NzcEYIDy4NPe3p6ADQ4Oypsb4ZUIdrapiQHJ/"
            "CI9GDw8PAh7YWGBzs7OhI7H45mqHo2UX82gJ0kTAEVa3d3dNDs7Kw3q6uoSJ6Z5GTYag22"
            "GMmt8TPT8XxeAnp4e+Q+jFLnAGFghZaygV+uKN484hZEBBX1/f+/"
            "xM6ICqVmOBZGwqqqqPrHRx/z8fPf29tbiJUH8f6XDw0PiVSZdfmOc6+lyFohi47/"
            "WVy6ENA/1l/"
            "XFg23zDhiRuqUXbBi1whJ9enqSQUWa7x3IcWHH0xDhLfUVYSpsWt6LMfZQwwX/"
            "wLVwWPG97osM9Wf7Df6GGOwnsP4BQFiPuOZ8wJUAAAAASUVORK5CYII=");

        native_tray_icon_id_t tray_id = native_tray_icon_get_id(g_tray_icon);
        printf("Tray ID: %ld\n", tray_id);

        char* title = native_tray_icon_get_title(g_tray_icon);
        if (title) {
            printf("Tray Title: %s\n", title);
            free_c_str(title);
        }

        // Create context menu
        g_context_menu = create_context_menu();

        // Set the context menu to the tray icon
        native_tray_icon_set_context_menu(g_tray_icon, g_context_menu);

        // Set up event listeners
        native_tray_icon_add_listener(g_tray_icon, NATIVE_TRAY_ICON_EVENT_CLICKED, on_tray_icon_clicked, NULL);
        native_tray_icon_add_listener(g_tray_icon, NATIVE_TRAY_ICON_EVENT_RIGHT_CLICKED, on_tray_icon_right_clicked, NULL);
        native_tray_icon_add_listener(g_tray_icon, NATIVE_TRAY_ICON_EVENT_DOUBLE_CLICKED, on_tray_icon_double_clicked, NULL);

        native_tray_icon_set_visible(g_tray_icon, true);
    } else {
        fprintf(stderr, "Failed to create tray.\n");
    }

    // Run the example app
    int result = native_run_example_app();

    // Cleanup
    if (g_context_menu) {
        native_menu_destroy(g_context_menu);
    }
    if (g_tray_icon) {
        native_tray_icon_destroy(g_tray_icon);
    }

    native_window_manager_shutdown();

    return result;
}
