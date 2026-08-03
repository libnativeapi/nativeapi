// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "geometry_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../foundation/geometry.h"

namespace {

native_point_t ToCPoint(const nativeapi::Point& value) {
  native_point_t result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

nativeapi::Point ToCppPoint(const native_point_t& value) {
  nativeapi::Point result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

native_size_t ToCSize(const nativeapi::Size& value) {
  native_size_t result = {};
  result.width = value.width;
  result.height = value.height;
  return result;
}

nativeapi::Size ToCppSize(const native_size_t& value) {
  nativeapi::Size result = {};
  result.width = value.width;
  result.height = value.height;
  return result;
}

native_rectangle_t ToCRectangle(const nativeapi::Rectangle& value) {
  native_rectangle_t result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

nativeapi::Rectangle ToCppRectangle(const native_rectangle_t& value) {
  nativeapi::Rectangle result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

}  // namespace

