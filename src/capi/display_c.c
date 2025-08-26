#include "display_c.h"
#include <stdlib.h>
#include <string.h>

native_display_t* native_display_create(void) {
  native_display_t* display =
      (native_display_t*)malloc(sizeof(native_display_t));
  if (!display) {
    return NULL;
  }

  // Initialize all fields to default values
  display->id = NULL;
  display->name = NULL;
  display->position.x = 0.0;
  display->position.y = 0.0;
  display->size.width = 0.0;
  display->size.height = 0.0;
  display->work_area.x = 0.0;
  display->work_area.y = 0.0;
  display->work_area.width = 0.0;
  display->work_area.height = 0.0;
  display->scale_factor = 0.0;
  display->is_primary = false;
  display->orientation = NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
  display->refresh_rate = 0;
  display->bit_depth = 0;
  display->manufacturer = NULL;
  display->model = NULL;
  display->serial_number = NULL;

  return display;
}

void native_display_destroy(native_display_t* display) {
  if (!display) {
    return;
  }

  // Free all allocated strings
  free(display->id);
  free(display->name);
  free(display->manufacturer);
  free(display->model);
  free(display->serial_number);

  // Free the display structure itself
  free(display);
}

static bool set_string_field(char** field, const char* value) {
  if (!field) {
    return false;
  }

  // Free existing string if any
  free(*field);
  *field = NULL;

  if (!value) {
    return true;  // Setting to NULL is valid
  }

  // Allocate and copy new string
  size_t len = strlen(value) + 1;
  *field = (char*)malloc(len);
  if (!*field) {
    return false;
  }

  strcpy(*field, value);
  return true;
}

bool native_display_set_id(native_display_t* display, const char* id) {
  if (!display) {
    return false;
  }
  return set_string_field(&display->id, id);
}

bool native_display_set_name(native_display_t* display, const char* name) {
  if (!display) {
    return false;
  }
  return set_string_field(&display->name, name);
}

bool native_display_set_manufacturer(native_display_t* display,
                                     const char* manufacturer) {
  if (!display) {
    return false;
  }
  return set_string_field(&display->manufacturer, manufacturer);
}

bool native_display_set_model(native_display_t* display, const char* model) {
  if (!display) {
    return false;
  }
  return set_string_field(&display->model, model);
}

bool native_display_set_serial_number(native_display_t* display,
                                      const char* serial_number) {
  if (!display) {
    return false;
  }
  return set_string_field(&display->serial_number, serial_number);
}

void native_display_set_position(native_display_t* display,
                                 double x,
                                 double y) {
  if (!display) {
    return;
  }
  display->position.x = x;
  display->position.y = y;
}

void native_display_set_size(native_display_t* display,
                             double width,
                             double height) {
  if (!display) {
    return;
  }
  display->size.width = width;
  display->size.height = height;
}

void native_display_set_work_area(native_display_t* display,
                                  double x,
                                  double y,
                                  double width,
                                  double height) {
  if (!display) {
    return;
  }
  display->work_area.x = x;
  display->work_area.y = y;
  display->work_area.width = width;
  display->work_area.height = height;
}

void native_display_set_scale_factor(native_display_t* display,
                                     double scale_factor) {
  if (!display) {
    return;
  }
  display->scale_factor = scale_factor;
}

void native_display_set_primary(native_display_t* display, bool is_primary) {
  if (!display) {
    return;
  }
  display->is_primary = is_primary;
}

void native_display_set_orientation(native_display_t* display,
                                    native_display_orientation_t orientation) {
  if (!display) {
    return;
  }
  display->orientation = orientation;
}

void native_display_set_refresh_rate(native_display_t* display,
                                     int refresh_rate) {
  if (!display) {
    return;
  }
  display->refresh_rate = refresh_rate;
}

void native_display_set_bit_depth(native_display_t* display, int bit_depth) {
  if (!display) {
    return;
  }
  display->bit_depth = bit_depth;
}
