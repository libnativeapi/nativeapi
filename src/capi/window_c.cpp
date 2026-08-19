// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "window_c.h"

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
#include "geometry_c.h"
#include "../foundation/color.h"
#include "color_c.h"
#include "../window.h"

// Conversion helpers between the C ABI types and their C++ originals.

inline native_title_bar_style_t ToCTitleBarStyle(nativeapi::TitleBarStyle value) {
  switch (value) {
    case nativeapi::TitleBarStyle::Normal:
      return NATIVE_TITLE_BAR_STYLE_NORMAL;
    case nativeapi::TitleBarStyle::Hidden:
      return NATIVE_TITLE_BAR_STYLE_HIDDEN;
    default:
      return NATIVE_TITLE_BAR_STYLE_NORMAL;
  }
}

inline nativeapi::TitleBarStyle ToCppTitleBarStyle(native_title_bar_style_t value) {
  switch (value) {
    case NATIVE_TITLE_BAR_STYLE_NORMAL:
      return nativeapi::TitleBarStyle::Normal;
    case NATIVE_TITLE_BAR_STYLE_HIDDEN:
      return nativeapi::TitleBarStyle::Hidden;
    default:
      return nativeapi::TitleBarStyle::Normal;
  }
}

inline native_visual_effect_t ToCVisualEffect(nativeapi::VisualEffect value) {
  switch (value) {
    case nativeapi::VisualEffect::None:
      return NATIVE_VISUAL_EFFECT_NONE;
    case nativeapi::VisualEffect::Blur:
      return NATIVE_VISUAL_EFFECT_BLUR;
    case nativeapi::VisualEffect::Acrylic:
      return NATIVE_VISUAL_EFFECT_ACRYLIC;
    case nativeapi::VisualEffect::Mica:
      return NATIVE_VISUAL_EFFECT_MICA;
    default:
      return NATIVE_VISUAL_EFFECT_NONE;
  }
}

inline nativeapi::VisualEffect ToCppVisualEffect(native_visual_effect_t value) {
  switch (value) {
    case NATIVE_VISUAL_EFFECT_NONE:
      return nativeapi::VisualEffect::None;
    case NATIVE_VISUAL_EFFECT_BLUR:
      return nativeapi::VisualEffect::Blur;
    case NATIVE_VISUAL_EFFECT_ACRYLIC:
      return nativeapi::VisualEffect::Acrylic;
    case NATIVE_VISUAL_EFFECT_MICA:
      return nativeapi::VisualEffect::Mica;
    default:
      return nativeapi::VisualEffect::None;
  }
}

inline native_point_t ToCPoint(const nativeapi::Point& value) {
  native_point_t result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

inline nativeapi::Point ToCppPoint(const native_point_t& value) {
  nativeapi::Point result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

inline native_size_t ToCSize(const nativeapi::Size& value) {
  native_size_t result = {};
  result.width = value.width;
  result.height = value.height;
  return result;
}

inline nativeapi::Size ToCppSize(const native_size_t& value) {
  nativeapi::Size result = {};
  result.width = value.width;
  result.height = value.height;
  return result;
}

inline native_rectangle_t ToCRectangle(const nativeapi::Rectangle& value) {
  native_rectangle_t result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

inline nativeapi::Rectangle ToCppRectangle(const native_rectangle_t& value) {
  nativeapi::Rectangle result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

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

native_window_t native_window_create(void) {
  try {
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::Window>());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_create");
    return 0;
  }
}

native_window_t native_window_create_with_native_window(void* native_window) {
  try {
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::Window>(native_window));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_create_with_native_window");
    return 0;
  }
}

native_window_id_t native_window_get_id(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return 0;
  }
  try {
    return self->GetId();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_id");
    return 0;
  }
}

void native_window_focus(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Focus();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_focus");
    return;
  }
}

void native_window_blur(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Blur();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_blur");
    return;
  }
}

bool native_window_is_focused(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsFocused();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_focused");
    return false;
  }
}

void native_window_show(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Show();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_show");
    return;
  }
}

void native_window_show_inactive(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->ShowInactive();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_show_inactive");
    return;
  }
}

void native_window_hide(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Hide();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_hide");
    return;
  }
}

bool native_window_is_visible(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsVisible();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_visible");
    return false;
  }
}

void native_window_maximize(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Maximize();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_maximize");
    return;
  }
}

void native_window_unmaximize(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Unmaximize();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_unmaximize");
    return;
  }
}

bool native_window_is_maximized(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsMaximized();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_maximized");
    return false;
  }
}

void native_window_minimize(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Minimize();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_minimize");
    return;
  }
}

void native_window_restore(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Restore();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_restore");
    return;
  }
}

bool native_window_is_minimized(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsMinimized();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_minimized");
    return false;
  }
}

void native_window_set_full_screen(native_window_t window, bool is_full_screen) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetFullScreen(is_full_screen);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_full_screen");
    return;
  }
}

bool native_window_is_full_screen(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsFullScreen();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_full_screen");
    return false;
  }
}

void native_window_set_bounds(native_window_t window, native_rectangle_t bounds) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto bounds_cpp = ToCppRectangle(bounds);
    self->SetBounds(bounds_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_bounds");
    return;
  }
}

native_rectangle_t native_window_get_bounds(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_rectangle_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetBounds();
    return ToCRectangle(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_bounds");
    native_rectangle_t result = {};
    return result;
  }
}

void native_window_set_content_bounds(native_window_t window, native_rectangle_t bounds) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto bounds_cpp = ToCppRectangle(bounds);
    self->SetContentBounds(bounds_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_content_bounds");
    return;
  }
}

native_rectangle_t native_window_get_content_bounds(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_rectangle_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetContentBounds();
    return ToCRectangle(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_content_bounds");
    native_rectangle_t result = {};
    return result;
  }
}

void native_window_set_size(native_window_t window, native_size_t size, bool animate) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto size_cpp = ToCppSize(size);
    self->SetSize(size_cpp, animate);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_size");
    return;
  }
}

native_size_t native_window_get_size(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_size_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetSize();
    return ToCSize(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_size");
    native_size_t result = {};
    return result;
  }
}

void native_window_set_content_size(native_window_t window, native_size_t size) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto size_cpp = ToCppSize(size);
    self->SetContentSize(size_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_content_size");
    return;
  }
}

native_size_t native_window_get_content_size(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_size_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetContentSize();
    return ToCSize(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_content_size");
    native_size_t result = {};
    return result;
  }
}

void native_window_set_minimum_size(native_window_t window, native_size_t size) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto size_cpp = ToCppSize(size);
    self->SetMinimumSize(size_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_minimum_size");
    return;
  }
}

native_size_t native_window_get_minimum_size(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_size_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetMinimumSize();
    return ToCSize(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_minimum_size");
    native_size_t result = {};
    return result;
  }
}

void native_window_set_maximum_size(native_window_t window, native_size_t size) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto size_cpp = ToCppSize(size);
    self->SetMaximumSize(size_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_maximum_size");
    return;
  }
}

native_size_t native_window_get_maximum_size(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_size_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetMaximumSize();
    return ToCSize(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_maximum_size");
    native_size_t result = {};
    return result;
  }
}

void native_window_set_resizable(native_window_t window, bool is_resizable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetResizable(is_resizable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_resizable");
    return;
  }
}

bool native_window_is_resizable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsResizable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_resizable");
    return false;
  }
}

void native_window_set_movable(native_window_t window, bool is_movable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetMovable(is_movable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_movable");
    return;
  }
}

bool native_window_is_movable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsMovable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_movable");
    return false;
  }
}

void native_window_set_minimizable(native_window_t window, bool is_minimizable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetMinimizable(is_minimizable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_minimizable");
    return;
  }
}

bool native_window_is_minimizable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsMinimizable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_minimizable");
    return false;
  }
}

void native_window_set_maximizable(native_window_t window, bool is_maximizable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetMaximizable(is_maximizable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_maximizable");
    return;
  }
}

bool native_window_is_maximizable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsMaximizable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_maximizable");
    return false;
  }
}

void native_window_set_full_screenable(native_window_t window, bool is_full_screenable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetFullScreenable(is_full_screenable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_full_screenable");
    return;
  }
}

bool native_window_is_full_screenable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsFullScreenable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_full_screenable");
    return false;
  }
}

void native_window_set_closable(native_window_t window, bool is_closable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetClosable(is_closable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_closable");
    return;
  }
}

bool native_window_is_closable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsClosable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_closable");
    return false;
  }
}

void native_window_set_window_control_buttons_visible(native_window_t window, bool is_visible) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetWindowControlButtonsVisible(is_visible);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_window_control_buttons_visible");
    return;
  }
}

bool native_window_is_window_control_buttons_visible(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsWindowControlButtonsVisible();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_window_control_buttons_visible");
    return false;
  }
}

void native_window_set_always_on_top(native_window_t window, bool is_always_on_top) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetAlwaysOnTop(is_always_on_top);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_always_on_top");
    return;
  }
}

bool native_window_is_always_on_top(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsAlwaysOnTop();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_always_on_top");
    return false;
  }
}

void native_window_set_position(native_window_t window, native_point_t point) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto point_cpp = ToCppPoint(point);
    self->SetPosition(point_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_position");
    return;
  }
}

native_point_t native_window_get_position(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_point_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetPosition();
    return ToCPoint(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_position");
    native_point_t result = {};
    return result;
  }
}

void native_window_center(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->Center();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_center");
    return;
  }
}

void native_window_set_title(native_window_t window, const char* title) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetTitle(std::string(title ? title : ""));
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_title");
    return;
  }
}

char* native_window_get_title(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return nullptr;
  }
  try {
    return to_c_str(self->GetTitle());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_title");
    return nullptr;
  }
}

void native_window_set_title_bar_style(native_window_t window, native_title_bar_style_t style) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetTitleBarStyle(ToCppTitleBarStyle(style));
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_title_bar_style");
    return;
  }
}

native_title_bar_style_t native_window_get_title_bar_style(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return (native_title_bar_style_t)NATIVE_TITLE_BAR_STYLE_NORMAL;
  }
  try {
    return ToCTitleBarStyle(self->GetTitleBarStyle());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_title_bar_style");
    return (native_title_bar_style_t)NATIVE_TITLE_BAR_STYLE_NORMAL;
  }
}

void native_window_set_has_shadow(native_window_t window, bool has_shadow) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetHasShadow(has_shadow);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_has_shadow");
    return;
  }
}

bool native_window_has_shadow(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->HasShadow();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_has_shadow");
    return false;
  }
}

void native_window_set_opacity(native_window_t window, float opacity) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetOpacity(opacity);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_opacity");
    return;
  }
}

float native_window_get_opacity(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return 0;
  }
  try {
    return self->GetOpacity();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_opacity");
    return 0;
  }
}

void native_window_set_visual_effect(native_window_t window, native_visual_effect_t effect) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetVisualEffect(ToCppVisualEffect(effect));
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_visual_effect");
    return;
  }
}

native_visual_effect_t native_window_get_visual_effect(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return (native_visual_effect_t)NATIVE_VISUAL_EFFECT_NONE;
  }
  try {
    return ToCVisualEffect(self->GetVisualEffect());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_visual_effect");
    return (native_visual_effect_t)NATIVE_VISUAL_EFFECT_NONE;
  }
}

void native_window_set_background_color(native_window_t window, native_color_t color) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    auto color_cpp = ToCppColor(color);
    self->SetBackgroundColor(color_cpp);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_background_color");
    return;
  }
}

native_color_t native_window_get_background_color(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    native_color_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetBackgroundColor();
    return ToCColor(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_get_background_color");
    native_color_t result = {};
    return result;
  }
}

void native_window_set_visible_on_all_workspaces(native_window_t window, bool is_visible_on_all_workspaces) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetVisibleOnAllWorkspaces(is_visible_on_all_workspaces);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_visible_on_all_workspaces");
    return;
  }
}

bool native_window_is_visible_on_all_workspaces(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsVisibleOnAllWorkspaces();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_visible_on_all_workspaces");
    return false;
  }
}

void native_window_set_ignore_mouse_events(native_window_t window, bool is_ignore_mouse_events) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetIgnoreMouseEvents(is_ignore_mouse_events);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_ignore_mouse_events");
    return;
  }
}

bool native_window_is_ignore_mouse_events(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsIgnoreMouseEvents();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_ignore_mouse_events");
    return false;
  }
}

void native_window_set_focusable(native_window_t window, bool is_focusable) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->SetFocusable(is_focusable);
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_set_focusable");
    return;
  }
}

bool native_window_is_focusable(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return false;
  }
  try {
    return self->IsFocusable();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_is_focusable");
    return false;
  }
}

void native_window_start_dragging(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->StartDragging();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_start_dragging");
    return;
  }
}

void native_window_start_resizing(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return;
  }
  try {
    self->StartResizing();
    return;
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_window_start_resizing");
    return;
  }
}

void* native_window_get_native_object(native_window_t window) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
  if (!self) {
    return nullptr;
  }
  return self->GetNativeObject();
}

void native_window_free(native_window_t window) {
  // The table invalidates the handle itself, so releasing an unknown or
  // already-released one is a no-op rather than a double free.
  nativeapi::HandleTable::GetInstance().Release(window);
}

void native_window_list_free(native_window_list_t* list) {
  if (!list || !list->windows) {
    return;
  }
  for (long i = 0; i < list->count; ++i) {
    nativeapi::HandleTable::GetInstance().Release(list->windows[i]);
  }
  delete[] list->windows;
  list->windows = nullptr;
  list->count = 0;
}

void native_window_list_release(native_window_list_t* list) {
  if (!list) {
    return;
  }
  delete[] list->windows;
  list->windows = nullptr;
  list->count = 0;
}

bool ToCWindowEvent(const nativeapi::WindowEvent& event, native_window_event_t* out) {
  if (!out) {
    return false;
  }
  *out = native_window_event_t{};
  out->window_id = event.GetWindowId();
  if (const auto* typed = dynamic_cast<const nativeapi::WindowFocusedEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_FOCUSED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::WindowBlurredEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_BLURRED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::WindowMinimizedEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_MINIMIZED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::WindowMaximizedEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_MAXIMIZED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::WindowRestoredEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_RESTORED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::WindowMovedEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_MOVED;
    out->data.moved.new_position = ToCPoint(typed->GetNewPosition());
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::WindowResizedEvent*>(&event)) {
    out->type = NATIVE_WINDOW_EVENT_TYPE_RESIZED;
    out->data.resized.new_size = ToCSize(typed->GetNewSize());
    return true;
  }
  return false;
}

void FreeCWindowEvent(native_window_event_t* value) {
  if (!value) {
    return;
  }
}

