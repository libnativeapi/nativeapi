#pragma once

#include <stdbool.h>
#include <stddef.h>
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
 * Opaque handle for image objects
 */
typedef void* native_image_t;

/**
 * Image operations
 */

/**
 * Create an image from a file path
 * @param file_path Path to the image file
 * @return Image handle, or NULL if loading failed
 */
FFI_PLUGIN_EXPORT
native_image_t native_image_from_file(const char* file_path);

/**
 * Create an image from base64-encoded data
 * @param base64_data Base64-encoded image data, with or without data URI prefix
 * @return Image handle, or NULL if decoding failed
 */
FFI_PLUGIN_EXPORT
native_image_t native_image_from_base64(const char* base64_data);

/**
 * Create an image from a platform-specific system icon
 * @param icon_name Platform-specific system icon name/identifier
 * @return Image handle, or NULL if icon not found
 */
FFI_PLUGIN_EXPORT
native_image_t native_image_from_system_icon(const char* icon_name);

/**
 * Destroy an image and release its resources
 * @param image The image to destroy
 */
FFI_PLUGIN_EXPORT
void native_image_destroy(native_image_t image);

/**
 * Get the size of an image in pixels
 * @param image The image
 * @param width Pointer to store the width (will be set to 0 if invalid)
 * @param height Pointer to store the height (will be set to 0 if invalid)
 */
FFI_PLUGIN_EXPORT
void native_image_get_size(native_image_t image, double* width, double* height);

/**
 * Get the image format string for debugging purposes
 * @param image The image
 * @return The image format (e.g., "PNG", "JPEG", "GIF"), or NULL if unknown
 * (caller must free)
 */
FFI_PLUGIN_EXPORT
char* native_image_get_format(native_image_t image);

/**
 * Convert an image to base64-encoded PNG data
 * @param image The image
 * @return Base64-encoded PNG data with data URI prefix (caller must free), or
 * NULL on error
 */
FFI_PLUGIN_EXPORT
char* native_image_to_base64(native_image_t image);

/**
 * Save an image to a file
 * @param image The image
 * @param file_path Path where the image should be saved
 * @return true if saved successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_image_save_to_file(native_image_t image, const char* file_path);

#ifdef __cplusplus
}
#endif
