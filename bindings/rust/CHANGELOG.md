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
