#include "color_c.h"
#include "../foundation/color.h"
#include <cstring>

using namespace nativeapi;

// Helper function to convert C++ Color to native_color_t
static native_color_t to_native_color(const Color& color) {
  native_color_t result;
  result.r = color.r;
  result.g = color.g;
  result.b = color.b;
  result.a = color.a;
  return result;
}

// Helper function to convert native_color_t to C++ Color
static Color from_native_color(const native_color_t& color) {
  return Color{color.r, color.g, color.b, color.a};
}

native_color_t native_color_from_rgba(unsigned char red,
                                      unsigned char green,
                                      unsigned char blue,
                                      unsigned char alpha) {
  native_color_t color;
  color.r = red;
  color.g = green;
  color.b = blue;
  color.a = alpha;
  return color;
}

bool native_color_from_hex(const char* hex, native_color_t* out_color) {
  if (!hex || !out_color) {
    return false;
  }

  try {
    Color color = Color::FromHex(hex);
    *out_color = to_native_color(color);
    return true;
  } catch (...) {
    return false;
  }
}

uint32_t native_color_to_rgba(native_color_t color) {
  Color cpp_color = from_native_color(color);
  return cpp_color.ToRGBA();
}

uint32_t native_color_to_argb(native_color_t color) {
  Color cpp_color = from_native_color(color);
  return cpp_color.ToARGB();
}

// Predefined color constants
extern "C" {
  const native_color_t NATIVE_COLOR_TRANSPARENT = {0, 0, 0, 0};
  const native_color_t NATIVE_COLOR_BLACK = {0, 0, 0, 255};
  const native_color_t NATIVE_COLOR_WHITE = {255, 255, 255, 255};
  const native_color_t NATIVE_COLOR_RED = {255, 0, 0, 255};
  const native_color_t NATIVE_COLOR_GREEN = {0, 255, 0, 255};
  const native_color_t NATIVE_COLOR_BLUE = {0, 0, 255, 255};
  const native_color_t NATIVE_COLOR_YELLOW = {255, 255, 0, 255};
  const native_color_t NATIVE_COLOR_CYAN = {0, 255, 255, 255};
  const native_color_t NATIVE_COLOR_MAGENTA = {255, 0, 255, 255};
}