#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "geometry_c.h"

/**
 * Display orientation enumeration
 */
typedef enum : int {
  NATIVE_DISPLAY_ORIENTATION_PORTRAIT = 0,
  NATIVE_DISPLAY_ORIENTATION_LANDSCAPE = 90,
  NATIVE_DISPLAY_ORIENTATION_PORTRAIT_FLIPPED = 180,
  NATIVE_DISPLAY_ORIENTATION_LANDSCAPE_FLIPPED = 270
} native_display_orientation_t;

/**
 * Representation of a display/monitor
 */
typedef struct {
  // Basic identification
  char* id;    // Unique identifier for the display
  char* name;  // Human-readable display name

  // Physical properties
  native_point_t position;  // Display position in virtual desktop coordinates
  native_size_t size;       // Full display size in logical pixels
  native_rectangle_t
      work_area;        // Available work area (excluding taskbars, docks, etc.)
  double scale_factor;  // Display scaling factor (1.0 = 100%, 2.0 = 200%, etc.)

  // Additional properties
  bool is_primary;  // Whether this is the primary display
  native_display_orientation_t orientation;  // Current display orientation
  int refresh_rate;  // Refresh rate in Hz (0 if unknown)
  int bit_depth;     // Color bit depth (0 if unknown)

  // Hardware information
  char* manufacturer;   // Display manufacturer
  char* model;          // Display model
  char* serial_number;  // Display serial number (if available)
} native_display_t;

/**
 * Representation of a list of displays
 */
typedef struct {
  native_display_t* displays;
  long count;
} native_display_list_t;

/**
 * Create a new display structure with default values
 * @return Pointer to newly allocated display structure, or NULL on failure
 */
native_display_t* native_display_create(void);

/**
 * Free a display structure and all its allocated strings
 * @param display Pointer to display structure to free
 */
void native_display_destroy(native_display_t* display);

/**
 * Set the ID of a display
 * @param display Pointer to display structure
 * @param id ID string (will be copied)
 * @return true on success, false on failure
 */
bool native_display_set_id(native_display_t* display, const char* id);

/**
 * Set the name of a display
 * @param display Pointer to display structure
 * @param name Name string (will be copied)
 * @return true on success, false on failure
 */
bool native_display_set_name(native_display_t* display, const char* name);

/**
 * Set the manufacturer of a display
 * @param display Pointer to display structure
 * @param manufacturer Manufacturer string (will be copied)
 * @return true on success, false on failure
 */
bool native_display_set_manufacturer(native_display_t* display,
                                     const char* manufacturer);

/**
 * Set the model of a display
 * @param display Pointer to display structure
 * @param model Model string (will be copied)
 * @return true on success, false on failure
 */
bool native_display_set_model(native_display_t* display, const char* model);

/**
 * Set the serial number of a display
 * @param display Pointer to display structure
 * @param serial_number Serial number string (will be copied)
 * @return true on success, false on failure
 */
bool native_display_set_serial_number(native_display_t* display,
                                      const char* serial_number);

/**
 * Set the position of a display
 * @param display Pointer to display structure
 * @param x X coordinate
 * @param y Y coordinate
 */
void native_display_set_position(native_display_t* display, double x, double y);

/**
 * Set the size of a display
 * @param display Pointer to display structure
 * @param width Width in logical pixels
 * @param height Height in logical pixels
 */
void native_display_set_size(native_display_t* display,
                             double width,
                             double height);

/**
 * Set the work area of a display
 * @param display Pointer to display structure
 * @param x X coordinate of work area
 * @param y Y coordinate of work area
 * @param width Width of work area
 * @param height Height of work area
 */
void native_display_set_work_area(native_display_t* display,
                                  double x,
                                  double y,
                                  double width,
                                  double height);

/**
 * Set the scale factor of a display
 * @param display Pointer to display structure
 * @param scale_factor Scale factor (1.0 = 100%, 2.0 = 200%, etc.)
 */
void native_display_set_scale_factor(native_display_t* display,
                                     double scale_factor);

/**
 * Set whether a display is primary
 * @param display Pointer to display structure
 * @param is_primary true if primary display, false otherwise
 */
void native_display_set_primary(native_display_t* display, bool is_primary);

/**
 * Set the orientation of a display
 * @param display Pointer to display structure
 * @param orientation Display orientation
 */
void native_display_set_orientation(native_display_t* display,
                                    native_display_orientation_t orientation);

/**
 * Set the refresh rate of a display
 * @param display Pointer to display structure
 * @param refresh_rate Refresh rate in Hz (0 if unknown)
 */
void native_display_set_refresh_rate(native_display_t* display,
                                     int refresh_rate);

/**
 * Set the bit depth of a display
 * @param display Pointer to display structure
 * @param bit_depth Color bit depth (0 if unknown)
 */
void native_display_set_bit_depth(native_display_t* display, int bit_depth);

#ifdef __cplusplus
}
#endif
