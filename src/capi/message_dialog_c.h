#pragma once

#include <stdbool.h>
#include <stdint.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dialog modality types
 */
typedef enum {
  NATIVE_DIALOG_MODALITY_NONE = 0,
  NATIVE_DIALOG_MODALITY_APPLICATION = 1,
  NATIVE_DIALOG_MODALITY_WINDOW = 2
} native_dialog_modality_t;

/**
 * Opaque handle for message dialog objects
 */
typedef void* native_message_dialog_t;

/**
 * MessageDialog operations
 */

/**
 * Create a new message dialog with title and message
 * @param title The dialog title
 * @param message The dialog message
 * @return Message dialog handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_message_dialog_t native_message_dialog_create(const char* title, const char* message);

/**
 * Destroy a message dialog and release its resources
 * @param dialog The message dialog to destroy
 */
FFI_PLUGIN_EXPORT
void native_message_dialog_destroy(native_message_dialog_t dialog);

/**
 * Set the dialog title
 * @param dialog The message dialog
 * @param title The title to set
 */
FFI_PLUGIN_EXPORT
void native_message_dialog_set_title(native_message_dialog_t dialog, const char* title);

/**
 * Get the dialog title
 * @param dialog The message dialog
 * @return The title string (caller must free), or NULL if dialog is invalid
 */
FFI_PLUGIN_EXPORT
char* native_message_dialog_get_title(native_message_dialog_t dialog);

/**
 * Set the dialog message
 * @param dialog The message dialog
 * @param message The message to set
 */
FFI_PLUGIN_EXPORT
void native_message_dialog_set_message(native_message_dialog_t dialog, const char* message);

/**
 * Get the dialog message
 * @param dialog The message dialog
 * @return The message string (caller must free), or NULL if dialog is invalid
 */
FFI_PLUGIN_EXPORT
char* native_message_dialog_get_message(native_message_dialog_t dialog);

/**
 * Set the modality of the dialog
 * @param dialog The message dialog
 * @param modality The modality type to set
 */
FFI_PLUGIN_EXPORT
void native_message_dialog_set_modality(native_message_dialog_t dialog,
                                        native_dialog_modality_t modality);

/**
 * Get the current modality setting of the dialog
 * @param dialog The message dialog
 * @return The current modality type
 */
FFI_PLUGIN_EXPORT
native_dialog_modality_t native_message_dialog_get_modality(native_message_dialog_t dialog);

/**
 * Open the dialog according to its modality setting
 * @param dialog The message dialog
 * @return true if the dialog was successfully opened, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_message_dialog_open(native_message_dialog_t dialog);

/**
 * Close the dialog programmatically
 * @param dialog The message dialog
 * @return true if the dialog was successfully closed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_message_dialog_close(native_message_dialog_t dialog);

#ifdef __cplusplus
}
#endif
