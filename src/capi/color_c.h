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
 * @struct native_color_t
 * @brief Representation of a color with RGBA components
 *
 * Each component is represented as an unsigned byte (0-255).
 * Alpha value: 0 = fully transparent, 255 = fully opaque
 */
typedef struct {
  unsigned char r;  /** Red component (0-255) */
  unsigned char g;  /** Green component (0-255) */
  unsigned char b;  /** Blue component (0-255) */
  unsigned char a;  /** Alpha component (0-255) */
} native_color_t;

/**
 * @brief Creates a color from RGBA values.
 *
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @param alpha Alpha component (0-255)
 * @return Color instance with specified values
 */
FFI_PLUGIN_EXPORT
native_color_t native_color_from_rgba(unsigned char red,
                                      unsigned char green,
                                      unsigned char blue,
                                      unsigned char alpha);

/**
 * @brief Creates a color from a hexadecimal string.
 *
 * Supports multiple hex color formats:
 * - "#RGB" - 3-digit hex (e.g., "#F00" = red)
 * - "#RGBA" - 4-digit hex with alpha
 * - "#RRGGBB" - 6-digit hex (e.g., "#FF0000" = red)
 * - "#RRGGBBAA" - 8-digit hex with alpha
 *
 * @param hex Hexadecimal color string (with or without '#' prefix)
 * @param out_color Pointer to store the resulting color
 * @return true if parsing succeeded, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_color_from_hex(const char* hex, native_color_t* out_color);

/**
 * @brief Converts the color to a 32-bit integer (RGBA format).
 *
 * @param color The color to convert
 * @return 32-bit unsigned integer in RGBA format (0xRRGGBBAA)
 */
FFI_PLUGIN_EXPORT
uint32_t native_color_to_rgba(native_color_t color);

/**
 * @brief Converts the color to a 32-bit integer (ARGB format).
 *
 * @param color The color to convert
 * @return 32-bit unsigned integer in ARGB format (0xAARRGGBB)
 */
FFI_PLUGIN_EXPORT
uint32_t native_color_to_argb(native_color_t color);

// Predefined color constants
FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_TRANSPARENT;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_BLACK;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_WHITE;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_RED;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_GREEN;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_BLUE;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_YELLOW;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_CYAN;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_MAGENTA;

#ifdef __cplusplus
}
#endif