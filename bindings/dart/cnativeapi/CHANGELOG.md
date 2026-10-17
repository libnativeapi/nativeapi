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

- Implement HarmonyOS application-local shortcuts through the explicit NativeShortcutHost component adapter, without probing or reserving global chords. Verify phone/computer real input, disabled states, C callback lifetime, worker registration and callback/event self-removal. Additional windows/native-only hosts and repeat/teardown audits remain open.

- Implement HarmonyOS application metadata, native image codecs, ASSET encrypted storage and clipboard image/local-file formats. ArkTS hosts initialize nativeapi and attach UIAbilityContext explicitly.

- Add HarmonyOS window management for initialized ArkTS hosts: main/child window lookup, asynchronous child creation, geometry, focus, visibility, full screen, native events and cancellable child closing. Host integration is explicit; Flutter does not attach WindowStage automatically.

## 0.5.3

- macOS: add `ShortcutReleasedEvent` for system key-up delivery and accept Fn / CapsLock accelerators (hotkey_manager migration).
- Add core clipboard text, HTML, images, file paths and change notifications on desktop and iOS; Android native hosts support monitoring and text/HTML through C++/JNI (no Flutter auto-initialization).

## 0.5.2

* Built from core with the Windows display name fix listed in nativeapi 0.5.2.

## 0.5.1

* `native_handle_finalize` and `native_user_data_revoke`: the native
  finalizers `nativeapi` attaches to handles and callbacks, which release a
  handle on the main thread and stop the core from calling a callback whose
  isolate group has shut down.

## 0.5.0

* Regenerated against core for the additions listed in nativeapi 0.5.0: window
  close capability, object listeners, close and quit request payloads, event
  decisions, and the new window and tray icon functions.

## 0.4.1

* Event loop shim for plain Dart programs (`src/event_loop*`, bound in
  `lib/src/event_loop.dart`): `cnativeapi_run_ui_thread`,
  `cnativeapi_start_event_loop`, `cnativeapi_pump_event_loop` and
  `cnativeapi_exit`, compiled into the same library as the core. They back
  `runNativeApp()` in `nativeapi`.

## 0.4.0

* `native_view_backend_t` and `native_view_is_backend_supported`,
  `native_view_set_default_backend`, `native_view_get_default_backend`,
  `native_view_get_backend`.
* Raw bindings for the view module (`native_view_*`, `native_label_*`,
  `native_button_*`, `native_text_field_*`, `native_image_view_*`,
  `native_window_get_content_view`) and `native_edge_insets_t`.
* cnativeapi is now a plain Dart package instead of a Flutter FFI plugin, so it
  works in Dart command-line apps as well as Flutter apps. A build hook
  (`hook/build.dart`) compiles the nativeapi core with the target's toolchain
  in place of the per-platform CMake, CocoaPods and SwiftPM projects.
* **Breaking:** the bindings are top-level `@Native` functions. The
  `CNativeApiBindings` class and the `cnativeApiBindings` instance are gone;
  call `native_window_create()` instead of
  `cnativeApiBindings.native_window_create()`. Enum-typed struct fields are
  now also readable as the Dart enum; the raw value is `<field>AsInt`.
* **Breaking:** every function taking a callback, and every struct with a
  callback field, takes a `native_release_user_data_t` after its `user_data`
  (`nullptr` if there is nothing to release). The core calls it exactly once,
  on the main thread, once it can no longer call the callback.
* The WinUI 3 backend on Windows (`NATIVEAPI_ENABLE_WINUI3`) is not built by
  the hook; the Win32 implementations are used.

* `native_window_set_content_under_title_bar`,
  `native_window_is_content_under_title_bar` and
  `native_window_is_content_under_title_bar_supported` are new, and
  `NATIVE_TITLE_BAR_STYLE_HIDDEN` now hides the window control buttons on macOS too.
  See the `nativeapi` changelog.
* `native_window_set_visual_effect` returns whether the effect is in force;
  `native_window_is_visual_effect_supported` is new, and so are the effects `MICA_ALT`,
  `HUD`, `POPOVER` and `MENU`. The effect is read from the native window, shows in a
  Flutter window on macOS, and on Windows Blur is a plain blur-behind that works on
  Windows 10. See the `nativeapi` changelog.
* `LaunchAtLogin` on Windows: an app running from an MSIX package gets a shortcut in the
  user's Startup folder, because the `Run` registry key an MSIX container writes is
  virtualized and never read. `IsEnabled()` also reads Explorer's `StartupApproved\Run`
  flag and `Enable()` clears it, so an entry the user switched off in Task Manager is not
  reported as enabled and enabling it again really enables it. The registry is written
  through the wide API, so a non-ASCII identifier or path survives.
* `LaunchAtLogin` on macOS: `setProgram()` naming the running application registers that
  application whatever the identifier says, instead of failing because the identifier does
  not name a bundled login item helper. Arguments are recorded but never delivered —
  `SMAppService` starts the app bundle, nothing else — where before any argument made
  `enable()` fail.

## 0.3.0

* **Breaking:** requires Flutter 3.47 / Dart 3.13.
* Android: the shared library is linked with 16 KB page alignment, so it loads on the
  Android 15 devices that use 16 KB memory pages.
* Linux: building no longer asks for `libayatana-appindicator3-dev`. The tray icon has been a
  StatusNotifierItem over D-Bus for a while; only the plugin's CMake file still required
  and linked the library.
* Update the embedded core: a translucent window background is see-through on Windows and
  Linux, `native_window_set_has_shadow` works on Linux, Linux hides GTK's header bar for
  `titleBarStyle = hidden` instead of un-decorating the window, and a Linux window's
  content size is measured without the shadow margin.

## 0.2.7

* Same as 0.2.6; released together with nativeapi 0.2.7 (there is no nativeapi 0.2.6).

## 0.2.6

* Update the embedded core and regenerate the C bindings.
* Add `native_tray_icon_set_icon_template` / `_is_icon_template`, `_set_icon_size` /
  `_get_icon_size`, `_set_icon_position` / `_get_icon_position` and
  `native_tray_icon_position_t`. macOS no longer forces tray icons to be template images.
* Add `native_window_set_parent_window` / `native_window_get_parent_window`, and the
  `NATIVE_WINDOW_EVENT_TYPE_CREATED` / `_CLOSED` window events.
* macOS: a window background color with alpha makes the window non-opaque.
* Windows: tray icons are added again when Explorer restarts; menu item clicks are emitted
  before the popup call returns, and its result says whether the menu was shown.
* Linux: one D-Bus connection per tray icon; window geometry measured by frame and content.
* Fix window handlers being called for menus that were already destroyed.

## 0.2.5

* Update the embedded core and regenerate the C bindings.
* Add `native_drag_source_*`, `native_drop_target_*` and `native_window_drag_session_*`
  for dragging files and text in and out of windows and for tear-off window drags, plus
  `native_window_manager_get_window_at_point`.
* Add `native_window_set_visible_in_taskbar` / `native_window_is_visible_in_taskbar`.
* Emit the window resized, moved, minimized, maximized and restored events, which the
  ABI declared the whole way through but never dispatched.
* Windows: track full screen as window state instead of comparing rectangles, fix
  `native_window_set_has_shadow`, the hidden title bar style, `native_window_is_minimized`,
  window hit testing and late drag starts, submenu detach/reattach and Top End placement.
* Linux: fix `native_window_manager_get_current` and `native_window_is_focused`,
  implement `native_window_start_dragging`, keep the application running instead of
  exiting at once, stop crashing on window-relative context menus, and survive monitors
  being replaced.
* macOS: keep the mouse gesture after a native window drag or resize starts.

## 0.2.4

* Windows: compile the C API directly into the plugin DLL to retain exported entry points.
* Compile Apple core sources as separate translation units to avoid duplicate definitions.
* Update the embedded core and generated C bindings.

## 0.2.3

* Add `native_window_set_non_activating` / `native_window_is_non_activating`
* macOS: `native_window_set_focusable` is now implemented (it was a no-op)

## 0.2.2

* Fix macOS build failure in 0.2.1: `redefinition of 'ToStdString'`. The macOS unity
  build compiles every platform `.mm` into one translation unit, and `app_info`,
  `device_info` and `launch_at_login` each defined that helper privately

## 0.2.1

* Update `cxx_impl` to core 5a5afc7
* Add `AppInfo` C bindings (name, identifier, version, build number)
* Add `DeviceInfo` C bindings (name, model, manufacturer, OS, kernel, architecture)
* Fix window focused/blurred events never being dispatched on macOS, Windows and Linux

## 0.2.0

* Regenerate the complete C bindings from the new code generator
* Unify the object identity/lifecycle model with a generational handle table
* Fix tray icon bounds on multi-display setups (macOS)
* Fix macOS global shortcuts never firing
* Rewrite EventEmitter locking and dispatch; add a main-thread dispatcher
* Add `UrlOpener` CanOpen support
* Require C++17 and propagate the requirement to consumers
* Remove the obsolete Python bindgen tooling

## 0.1.4

* Fix macOS menu item disabled state not being respected
* Fix Windows window `SetMinimumSize`/`SetMaximumSize` not working
* Add per-monitor DPI scaling for Windows display and window geometry
* Track menu item enabled state and update flags on Windows
* Adjust Linux tray icon menu trigger handling

## 0.1.3

* Remove duplicate macOS deployment target build setting from the podspec
* Update storage example Darwin integration to use Swift Package Manager

## 0.1.2

* Add LaunchAtLogin C bindings

## 0.1.1

* Add multi-platform FFI C bindings (Android, iOS, Linux, macOS, Windows)
* Migrate iOS and macOS native bindings to SwiftPM
* Add UrlOpener C bindings
* Add autostart and global shortcuts C bindings
* Add window visual effects and color C bindings
* Add title bar style and control button C bindings
* Introduce bindgen code generation toolchain

## 0.1.0

* Initial release
