## 0.5.1

* Linux: `TrayIcon::set_title` sets the StatusNotifierItem label, drawn next to
  the icon by hosts that show one (GNOME's AppIndicator extension), instead of
  the tooltip's heading. The tooltip shows the tooltip alone, and the item's
  `Title` is the application name.
* The C ABI gains `native_handle_finalize` and `native_user_data_revoke`, for
  bindings whose runtime frees handles and callbacks from a garbage
  collector's native finalizer.

## 0.5.0

* `Window::set_ignore_mouse_events(ignore, forward)` takes a second argument
  and returns whether the native policy was applied: macOS, Windows and X11 can
  forward hover movement while clicks pass through, and Linux implements
  pass-through. `is_mouse_move_forwarding_enabled` and
  `is_mouse_move_forwarding_supported` inspect forwarding. **Breaking.**
* `ApplicationEvent::QuitRequested` carries an `EventRequestRef`, and the new
  `WindowEvent::CloseRequested` one too: `cancel` it, or `defer` it into an
  `EventDecision` to `accept` or `cancel` later. `Window::close` asks the
  listeners first; `Window::is_close_supported` says whether it applies.
  **Breaking** for code matching `QuitRequested` without fields.
* `Window::add_listener` and `remove_listener`: listeners on one window.
* `WindowEvent::PropertyChanged` with `WindowProperty`: a window's title,
  resizable / movable / minimizable / maximizable / full-screenable / closable
  state, control button visibility, always-on-top / on-bottom and title bar
  style report each change, whoever made it.
* `Window::occlusion_state` and `WindowEvent::OcclusionChanged`: whether any
  part of a window can be seen. macOS uses the system's state, Windows computes
  it from the windows above, Linux knows only hidden and minimized windows.
* `Window::set_maximize_button_bounds`: where a custom title bar draws its
  maximize button, so Windows 11 opens the snap layouts on it.
* `Window::show_system_menu`: the native system window menu on Windows and
  supporting Linux window managers.
* `Window::perform_title_bar_double_click`: the macOS title-bar double-click
  preference (Maximize, Minimize or None) for custom title bars; a hidden
  title bar's empty band on macOS performs it by itself.
* `Window::set_content_protection` and `is_content_protected`: native capture
  policy on Windows and macOS.
* `Window::set_corner_preference` and `WindowCornerPreference`: Windows 11's
  corner rounding policy.
* `TrayIcon::with_identifier` and `identifier`: a persistent tray host name
  (the SNI ID on Linux, `autosaveName` on macOS).
* Windows and Linux: `Window::set_focusable` and `set_non_activating` take
  effect; disabling focus releases a keyboard focus the window holds.
* `Window::focus` is reliable, and `blur` hands the focus back to where it was.
* Linux: window hooks and shortcuts shut down safely; X11 always-on-top state
  is read back from the window manager; one window ID on both GTK objects.

## 0.4.1

* `TrayIcon::set_content_view` and `content_view`: any `View` in place of the
  icon and title, on macOS. Recorded only on Windows and Linux.
* `Application::show`, `hide` and `is_visible`: application-level hiding on
  macOS, which the Dock understands. They return false on other platforms.
* `WindowEvent::EnteredFullScreen` and `ExitedFullScreen`.
* Row and Column containers size to their content.
* Windows: screen coordinates are one logical space across monitors with
  different scale factors; display positions change where monitors have
  different factors, and `TrayIcon` bounds and menu positions are logical
  instead of physical pixels.
* Windows: quitting destroys the windows before the loop ends; a full-screen
  window keeps no title bar when its title bar style is set.
* Windows: a window with a hidden title bar keeps the system's frame instead of
  a classic-theme border; without a shadow, with a custom shadow or with a shape
  it has no frame at all and resizes from every edge and corner.
* Linux: a hidden title bar's shadow gutter no longer leaves an empty band
  around the content or inflates the window on Hyprland.
* macOS: `View::create` no longer crashes.

## 0.4.0

The first release built from the nativeapi repository, where the Rust binding now
lives next to the other bindings and is generated from the C++ headers of
[nativeapi-core](https://github.com/libnativeapi/nativeapi-core).

* `nativeapi` covers the whole core API: `Application`, `Window` and
  `WindowManager` (title bar styles, visual effects, custom shadows, window and
  input shapes, drag sessions), `TrayIcon`, `Menu`, `Display` and
  `DisplayManager`, keyboard monitoring and global shortcuts, file and message
  dialogs, drag and drop, notifications, launch at login, preferences and secure
  storage, URL opening, accessibility, app and device info, and native views
  (`View`, `Label`, `Button`, `TextField`, `ImageView`).
* Callbacks are released once the core lets them go: a listener when it is
  removed or its emitter destroyed, a replaced callback, and any callback whose
  registration failed.
* `cnativeapi` carries a copy of the core sources and builds them with CMake
  from `build.rs`; building needs CMake and a C++17 compiler, and on Linux GTK 3,
  X11 and XInput development packages.
* The `nativeapi` crate no longer ships a placeholder `nativeapi` binary.

## 0.0.1

* Initial release.
