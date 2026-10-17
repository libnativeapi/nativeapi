## Unreleased

- Confirm HarmonyOS immersive-layout requests against the real SDK result and live layout flag on workers, reject overlapping alias requests, and cancel pending work on native destruction. Keep computer floating-main no-op and native-caption inset limits explicit.

- Implement HarmonyOS fullscreen-mode policy with preserved floating/split bits, an explicit host override baseline and primary Stage fallback. Serialize UI/worker requests, keep policies on SDK refusal, and cancel pending requests on native destruction. The SDK couples fullscreen and maximize; unknown child policy queries return false.

- Use native HarmonyOS fullscreen modes for eligible freeform child windows, restore their previous maximized/floating state and report confirmed SDK state through C++/C queries and fullscreen events. Preserve phone SDK layout restrictions.

- Implement HarmonyOS custom title-bar double-click actions through public SDK maximize/recover, including eligible SDK children. Check live free-window support to reject phone SDK no-op success, respect applied maximize policies and invalid modes, reject overlapping requests and cancel pending work on native destruction; UI submissions are asynchronous and workers wait for SDK completion.

- Implement independent HarmonyOS minimize/maximize/close caption-button policies, composed with collective visibility and title-bar styles, with C++/C property events. These affect caption buttons only: native title-bar double-click can still maximize, and primary programmatic Close retains its existing restriction.

- Recover HarmonyOS custom shadows after native helper destruction; verify primary-window rendering and decoration/default restoration on computers, and SDK refusal on phones.

- Implement HarmonyOS custom window shadows through an optional Stage companion: core-rendered colors, blur and offset, copied C++/C configuration, click-through helpers, live geometry and visibility, clipped shapes and native closure cleanup.

- Return transparent colors for malformed hexadecimal input without throwing; preserve combined keyboard modifiers across the C ABI. Retain shared windows in relative positioning strategies and expose the owning relative-window getter in all bindings. C++ callers must pass `Relative(window)` instead of `Relative(*window)`.

- Display HarmonyOS native-view tooltips from real hover events through the optional NativeTooltipHost companion. Public SDK popups preserve focus and underlying clicks; updates, clearing, hidden/disabled ancestors, removal, node replacement and ContentSlot lifecycle cancel stale presentations.

- Implement HarmonyOS child-window reparenting through the public SDK. Read actual parent relationships, including embedding-host changes; reject cycles, invalid handles and overlapping requests, and wait for SDK completion on workers. Verify native/C behavior and real parent destruction on phone/computer simulators.

- Query the actual HarmonyOS visible-window stack for window hit testing on workers, with nonblocking completed snapshots on UI and per-display caches. Verify focus-independent child layers, SDK raises, exclusions and destruction through C++/C on phone/computer; restore the device-information test export and rerun storage/metadata acceptance.

- Preserve HarmonyOS Preferences removal semantics for missing keys. Share each scope's native store and serialize compound operations, closing the SDK handle only after the final owner; verify C++/C, SDK interoperability, concurrency, limits and real process persistence on phone/computer simulators.

- Read live HarmonyOS user device names through the public Settings SDK with the documented market-name fallback. Preserve unavailable distribution names as empty, and report the process build ABI independently of the kernel machine. Verify all seven getters through ordinary C++/C UI and worker reads, real restored name changes and process restarts on phone/computer simulators.

- Implement HarmonyOS public-NDK key press/release monitoring with shared native registration, UI delivery, stopped-epoch invalidation and guarded destruction. Both phone/computer system-app debug fixtures pass real input and C++/C lifecycle checks; normal apps remain denied even with the tested ACL. Modifier-state events and remaining permission/teardown audits stay open.

- Read actual HarmonyOS self startup status through the public SDK on workers, with nonblocking last-completed UI snapshots. Resolve default metadata after host attachment; keep programmatic startup controls unavailable and record display names as metadata only.

- Implement HarmonyOS application-local shortcuts through the explicit NativeShortcutHost component adapter, without probing or reserving global chords. Verify phone/computer real input, disabled states, C callback lifetime, worker registration and callback/event self-removal.

- Implement HarmonyOS application metadata, native image codecs, ASSET encrypted storage and clipboard image/local-file formats. ArkTS hosts initialize nativeapi and attach UIAbilityContext explicitly.

* Add HarmonyOS window management for initialized ArkTS hosts: attached main/child windows, geometry, focus/visibility/full screen, native events and cancellable child closing. The host integrates WindowStage explicitly.

* `TrayIcon::set_icon_template` now follows system tray colours on Windows and Linux as well as macOS, preserving the source image.
* macOS: the title bar's close button is a cancellable close request again.
  On macOS 26 it bypassed `WindowEvent::CloseRequested` and closed the window
  even when a listener cancelled.

## 0.5.2

* Windows: `Display::name` is the monitor's name ("DELL U2720Q", or "Generic PnP
  Monitor" where Windows has none) instead of the adapter output
  `\\.\DISPLAY1`.

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
