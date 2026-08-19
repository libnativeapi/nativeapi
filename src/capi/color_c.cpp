// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "color_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../foundation/color.h"

// Conversion helpers between the C ABI types and their C++ originals.

inline native_color_t ToCColor(const nativeapi::Color& value) {
  native_color_t result = {};
  result.r = value.r;
  result.g = value.g;
  result.b = value.b;
  result.a = value.a;
  return result;
}

inline nativeapi::Color ToCppColor(const native_color_t& value) {
  nativeapi::Color result = {};
  result.r = value.r;
  result.g = value.g;
  result.b = value.b;
  result.a = value.a;
  return result;
}

const native_color_t NATIVE_COLOR_TRANSPARENT = ToCColor(nativeapi::Color::Transparent);

const native_color_t NATIVE_COLOR_BLACK = ToCColor(nativeapi::Color::Black);

const native_color_t NATIVE_COLOR_WHITE = ToCColor(nativeapi::Color::White);

const native_color_t NATIVE_COLOR_RED = ToCColor(nativeapi::Color::Red);

const native_color_t NATIVE_COLOR_GREEN = ToCColor(nativeapi::Color::Green);

const native_color_t NATIVE_COLOR_BLUE = ToCColor(nativeapi::Color::Blue);

const native_color_t NATIVE_COLOR_YELLOW = ToCColor(nativeapi::Color::Yellow);

const native_color_t NATIVE_COLOR_CYAN = ToCColor(nativeapi::Color::Cyan);

const native_color_t NATIVE_COLOR_MAGENTA = ToCColor(nativeapi::Color::Magenta);

native_color_t native_color_from_rgba(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) {
  try {
    const auto cpp_result = nativeapi::Color::FromRGBA(red, green, blue, alpha);
    return ToCColor(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_color_from_rgba");
    native_color_t result = {};
    return result;
  }
}

native_color_t native_color_from_hex(const char* hex) {
  try {
    const auto cpp_result = nativeapi::Color::FromHex(hex);
    return ToCColor(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_color_from_hex");
    native_color_t result = {};
    return result;
  }
}

unsigned int native_color_to_rgba(native_color_t color) {
  try {
    const auto self = ToCppColor(color);
    return self.ToRGBA();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_color_to_rgba");
    return 0;
  }
}

unsigned int native_color_to_argb(native_color_t color) {
  try {
    const auto self = ToCppColor(color);
    return self.ToARGB();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_color_to_argb");
    return 0;
  }
}

