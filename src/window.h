#pragma once
#include <string>
#include "geometry.h"

namespace nativeapi {

typedef long WindowID;

class Window {
 public:
  Window();
  Window(void* window);
  virtual ~Window();

  WindowID id;

  void Focus();
  void Blur();
  bool IsFocused() const;
  void Show();
  void ShowInactive();
  void Hide();
  bool IsVisible() const;
  void Maximize();
  void Unmaximize();
  bool IsMaximized() const;
  void Minimize();
  void Restore();
  bool IsMinimized() const;
  void SetFullScreen(bool is_full_screen);
  bool IsFullScreen() const;
  // void SetBackgroundColor(Color color);
  // Color GetBackgroundColor() const;
  void SetBounds(Rectangle bounds);
  Rectangle GetBounds() const;
  void SetSize(Size size, bool animate);
  Size GetSize() const;
  void SetContentSize(Size size);
  Size GetContentSize() const;
  void SetMinimumSize(Size size);
  Size GetMinimumSize();
  void SetMaximumSize(Size size);
  Size GetMaximumSize();
  void SetResizable(bool is_resizable);
  bool IsResizable() const;
  void SetMovable(bool is_movable);
  bool IsMovable() const;
  void SetMinimizable(bool is_minimizable);
  bool IsMinimizable() const;
  void SetMaximizable(bool is_maximizable);
  bool IsMaximizable() const;
  void SetFullScreenable(bool is_full_screenable);
  bool IsFullScreenable() const;
  void SetClosable(bool is_closable);
  bool IsClosable() const;
  void SetAlwaysOnTop(bool is_always_on_top);
  bool IsAlwaysOnTop();
  void SetPosition(Point point);
  Point GetPosition();
  void SetTitle(std::string title);
  std::string GetTitle();
  void SetHasShadow(bool has_shadow);
  bool HasShadow() const;
  void SetOpacity(float opacity);
  float GetOpacity() const;
  void SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces);
  bool IsVisibleOnAllWorkspaces() const;
  void SetIgnoreMouseEvents(bool is_ignore_mouse_events);
  bool IsIgnoreMouseEvents() const;
  void SetFocusable(bool is_focusable);
  bool IsFocusable() const;
  void StartDragging();
  void StartResizing();

  void* GetNSWindow() const;

 private:
  class Impl;
  Impl* pimpl_;
};

}  // namespace nativeapi