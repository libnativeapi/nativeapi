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

- Route HarmonyOS system title-bar closes through shared cancellable/deferred C++ and C window requests. Approval continues the original SDK close; repeated clicks coalesce, and removing observers or destroying a pending window cannot revive stale votes.

- Implement HarmonyOS programmatic drag sources for initialized ArkTS hosts, including text/files, custom/default previews, guarded UI completion and safe release during an ongoing drag. Add `DragSource.isPositionSupported()` and `isOperationSelectionSupported()` across native platforms and bindings. HarmonyOS reports false for both missing SDK capabilities; completion preserves actual Copy/Move/None results.

- Implement HarmonyOS native ArkUI drop destinations on mounted NativeAPI content roots. Copy drops deliver text and readable file paths; Move/Link remain refused. Isolate native view and drag event receivers, and allow target removal/destruction during delivery.

- Implement HarmonyOS public-NDK global shortcut registration with Ctrl/Alt/Shift side variants, alias/system conflict checks and guarded UI delivery. Failed removal retains the registration for retry; failed registration notifications can re-enter the manager without a dispatcher. Release events and complete keyboard monitoring remain open.

- Validate HarmonyOS clipboard file URI reads against actual file type and app readability, and normalize foreign sandbox paths before accepting them. Verify bidirectional text, HTML, pixels and real file bytes with a separate SDK-only application.

- Implement HarmonyOS display change, addition, removal and available-area observers through the public NDK, with main-thread delivery and invalidation after the last unsubscription. Preserve display identities when native enumeration fails.

- Query HarmonyOS cursor positions through the public Input Kit API, convert physical display coordinates to logical coordinates, and return the documented origin on SDK refusal or missing pointer/display geometry. Focus authorization and an actual pointer device are required.

- Implement HarmonyOS main-window content aspect constraints through the API 21+ nonpersistent SDK operation. Only user edge resizing is constrained; worker calls wait for completion, queued requests preserve order and rejected operations preserve the last successful value.

- Implement HarmonyOS native window-control visibility and live C++/C property events on supported freeform main windows. Hidden title bars hide the buttons; explicitly showing them afterwards overrides that default. Unsupported phone/child windows remain safely hidden.

- Implement HarmonyOS native window dragging for initialized ArkTS hosts through the public startMoving API. Invoke from the active touch-down/move callback; main windows require freeform mode.

- Implement HarmonyOS child-window polygon masks and rectangle restoration through the public SDK, with shared C++/C state, copied builders and real SDK completion on worker threads. Requires API 24+; main windows and independent input polygons remain unsupported.

- Implement HarmonyOS owning-Ability lifecycle events/state, cancellable C++/C ABI quit confirmation followed by real Ability termination, primary-window selection, application multi-instance metadata and application light/dark/system appearance. Hosts forward the actual Ability lifecycle callbacks; the Ability owns its loop, so Application.run returns -1.

- Fix HarmonyOS accessibility queries to read actual system extension state through the public SDK on UI and worker threads. Enable no longer sets a local success flag; activating an extension requires a system-only API.

- Implement HarmonyOS system notifications through public authorization, publish/cancel and WantAgent APIs, including tag replacement and native/C ABI activation events. Worker operations require the ArkTS UI loop; hosts forward onCreate/onNewWant through the OHOS bridge.

- Implement HarmonyOS HTTP/HTTPS handler queries and URL dispatch through the public SDK. Declare querySchemes for capability queries; Open waits on a worker for the actual launch result and reports SDK failures.

- Implement HarmonyOS system document pickers using the public SDK. Computer open, multiple-file, folder and save paths are verified; phone save, single-open and user cancellation are verified, with multi-file and folder acceptance still under investigation. Open waits on a worker and returns authorized document URIs for the system file APIs; programmatic Close is unavailable.

- Implement HarmonyOS native message dialogs with buttons, input, checkbox, progress, live updates and lifecycle cleanup. Modal Open requires a worker while the ArkTS UI thread keeps handling events.

- Implement HarmonyOS application metadata, native image codecs, ASSET encrypted storage and clipboard image/local-file formats. ArkTS hosts initialize nativeapi and attach UIAbilityContext explicitly.

- Add HarmonyOS window management for initialized ArkTS hosts: main/child window lookup, asynchronous child creation, geometry, focus, visibility, full screen, native events and cancellable child closing. Host integration is explicit; Flutter does not attach WindowStage automatically.

## 0.5.3

- `TrayIcon.isIconTemplate` now follows system tray colours on Windows and Linux as well as macOS, without changing the source image.
- macOS: add `ShortcutReleasedEvent` for system key-up delivery and accept Fn / CapsLock accelerators (hotkey_manager migration).
- Add core clipboard text, HTML, images, file paths and change notifications on desktop and iOS; Android native hosts support monitoring and text/HTML through C++/JNI (no Flutter auto-initialization).
- macOS: the title bar's close button is a cancellable close request again.
  On macOS 26 it bypassed `WindowCloseRequestedEvent` and closed the window
  even when a listener cancelled.

## 0.5.2

* Windows: `Display.name` is the monitor's name ("DELL U2720Q", or "Generic PnP
  Monitor" where Windows has none) instead of the adapter output
  `\\.\DISPLAY1` (leanflutter/screen_retriever#19).

## 0.5.1

* Objects created from Dart no longer outlive a Flutter hot restart
  (leanflutter/tray_manager#106): their handles are released by a
  `NativeFinalizer`, which also runs when the isolate group shuts down, so the
  old tray icons, menus and the like go away instead of piling up. Native
  classes implement `Finalizable`.
* A hot restart no longer crashes the app on the next event: callbacks the old
  isolate registered are revoked when it shuts down, so the core stops calling
  them.
* Linux: `TrayIcon.setTitle()` sets the StatusNotifierItem label, drawn next to
  the icon by hosts that show one (GNOME's AppIndicator extension), instead of
  the tooltip's heading. The tooltip shows `setTooltip()` alone, and the item's
  `Title` is the application name.

## 0.5.0

* `Window.setMaximizeButtonBounds()`: tell the window where a custom title
  bar draws its maximize button, so Windows 11 opens the snap layouts on it
  again. Hover, press and release still reach the app's button. Windows
  only; `Window.isMaximizeButtonBoundsSupported()` says whether it applies.

* `Window.occlusionState` and `WindowOcclusionChangedEvent`: whether any part
  of a window can be seen (`visible`, `occluded`, or `unknown` where the
  platform cannot tell), to pause rendering or video nobody sees. macOS uses
  the system's own state; Windows computes it from the windows above; Linux
  knows only hidden and minimized windows.

* `WindowPropertyChangedEvent`: a window's title, resizable / movable /
  minimizable / maximizable / full-screenable / closable state, control button
  visibility, always-on-top / on-bottom and title bar style report each change,
  whether a setter on any wrapper or the system or other code changed the
  native window. Read the new value through the getter `WindowProperty` names.

* Apps that also target the web can depend on nativeapi again
  (leanflutter/tray_manager#108). On the web `package:nativeapi` resolves to a
  pure-Dart mirror without `dart:ffi`: the same classes, enums, value types and
  events, minus the members that expose the C form (`raw`, `fromNative`,
  `allocNative`, `nativeObject`, raw pointers). Every platform call there
  throws `UnsupportedError`, so guard it with `kIsWeb`.

* Event listeners run on their registering isolate, including native events
  from foreign UI threads. Callbacks may return a Future; event payloads and
  borrowed handles remain valid until completion. Removing a listener skips
  queued callbacks that have not started.

* Add shared `EventRequest` and owned `EventDecision` primitives for cancellation
  and deferred confirmation. Programmatic `Application.quit()` now waits for
  listener Futures and deferred decisions; cancellation leaves the application
  running. macOS native quit queries now share this confirmation; approval
  continues the host decision, including Flutter's asynchronous exit callback.
  Mandatory native termination invalidates old votes and cannot be vetoed.

* Add object-level `Window` listeners and `Window.close()` confirmation on macOS,
  Windows and GTK (X11 or Wayland). Native aliases share pending votes; approval
  continues the host close handler, and forced native destruction invalidates
  old decisions. Mobile platforms report close as unsupported.

* On macOS, initializing `Application` preserves an existing host delegate and
  activation policy. Native lifecycle listeners observe quit requests and preserve
  the host's decision without bypassing Flutter's exit confirmation.

* On macOS, empty backgrounds in a hidden window's original title-bar band
  perform the preferred double-click action automatically. Controls and custom
  mouse handlers keep their gestures. The native Fill action is requested when
  the OS provides it.

* Adds `Window.performTitleBarDoubleClick()` for custom title bars, respecting
  the macOS Maximize, Minimize and None preferences.

* `Window.setIgnoreMouseEvents(ignore, forward)` now returns whether the native
  policy was applied. Linux implements pass-through and restores input shapes;
  macOS, Windows and X11 can forward hover movement while clicks pass through.
  Use `isMouseMoveForwardingEnabled` and `isMouseMoveForwardingSupported()` to
  inspect forwarding. Wayland rejects forwarding without changing the policy.
  This method replaces the `isIgnoreMouseEvents` setter; the getter is retained.

* `Window.showSystemMenu(Point)` and `Window.isSystemMenuSupported()`: native
  system window menu on Windows and supporting Linux window managers. Positions
  are logical pixels relative to the content area. Windows keeps command states
  current; Wayland requires an active pointer press in the target window.

* `Window.setContentProtection()`, `isContentProtected` and
  `Window.isContentProtectionSupported()`: native capture policy on Windows and
  macOS. Recent macOS ScreenCaptureKit ignores the legacy sharing policy; Linux
  and mobile platforms return false.

* `Window.setCornerPreference()`, `cornerPreference` and
  `isCornerPreferenceSupported()`: Windows 11's compositor corner policy
  (default, square, rounded or small rounding), with the native title bar kept.
  Returns false on other platforms; the getter reports the accepted preference,
  not the radius Windows chooses for maximized, snapped or shaped windows.
* `TrayIcon.createWithIdentifier()` and the read-only `identifier`: a persistent
  tray host name, set before registration. Linux defaults to the GApplication ID
  or executable name, with suffixes for additional live icons; macOS uses
  `NSStatusItem.autosaveName`. Windows records the name. The numeric object ID
  remains separate.
* Windows and Linux: `Window.isFocusable` and `isNonActivating` take effect;
  disabling focus releases a keyboard focus the window holds.
* `Window.focus()` is reliable, and `blur()` hands the focus back to where it
  was.
* Linux: window hooks and shortcuts shut down safely; X11 always-on-top state
  is read back from the window manager; one window ID on both GTK objects.

## 0.4.1

* `TrayIcon.contentView`: any `View` in place of the icon and title, on macOS.
  The menu bar item is as wide as the view (its `preferredSize`, or what a Row
  or Column needs) and follows it as it changes; clicks outside buttons and
  text fields still reach the tray icon's events. Recorded only on Windows and
  Linux.
* `runNativeApp()`: windows and native views from a plain Dart program, no
  Flutter needed. The app runs in an isolate of its own on the platform's UI
  thread, with Dart's timers, futures and streams working in between; on
  macOS that is the process's first thread, taken over from the Dart VM.
  `nativeapi_flutter` does not re-export it.
* Row and Column containers size to their content: a container's
  `intrinsicSize` is what its subviews need, so nested rows and columns no
  longer collapse to zero without a `preferredSize`.
* macOS: `View.create()` no longer crashes (the container was released as soon
  as it was created); buttons are no longer clipped in a process linked against
  an older macOS SDK, such as the Dart VM; destroying a view that holds the
  focused text field no longer emits `ViewBlurredEvent` into the tree being
  torn down, which crashed at exit.
* `Application.quit()` from a host-run loop now emits `ApplicationExitingEvent`
  on Windows and Linux as well, and ends the process on Linux, as documented.
  On Windows it destroys the windows before the loop ends, so a Flutter app's
  engine shuts down cleanly instead of crashing in its teardown.
* Windows: screen coordinates are one logical space across monitors with
  different scale factors, laid out side by side as Chromium does. A window's
  position and bounds survive being saved and restored on any monitor, and
  moving a window onto a monitor with another factor keeps the size asked for.
  Display positions change where monitors have different factors (they
  overlapped before); `TrayIcon.bounds` and menu positions are logical now,
  like everything else, instead of physical pixels.
* `Application.show()`, `Application.hide()` and `Application.isVisible()`:
  application-level hiding on macOS (`-[NSApplication hide:]`), which the Dock
  understands, unlike hiding every window. They return false on other
  platforms.
* `WindowEnteredFullScreenEvent` and `WindowExitedFullScreenEvent`, on macOS
  and Linux whoever changes the state, on Windows for `Window.isFullScreen`.
* Windows: setting `Window.titleBarStyle` on a full-screen window no longer
  puts a title bar on it; leaving full screen shows the title bar asked for.
* Windows: a window with a hidden title bar keeps the system's frame (thin
  border, shadow and rounded corners) instead of a classic-theme border on its
  sides and bottom. Without a shadow, with a custom shadow or with a shape it
  has no frame at all and resizes from every edge and corner, also through a
  Flutter view that covers it, without the classic frame flickering in while
  resizing. The content stays in place when the frame comes or goes.
* Linux: a hidden title bar's shadow gutter no longer leaves an empty band
  around the content, and no longer inflates the window on compositors that
  report every window as tiled, such as Hyprland.

## 0.4.0

* `ViewBackend` and `View.isBackendSupported`, `View.setDefaultBackend`,
  `View.getDefaultBackend` and `View.backend`: on Windows builds with the WinUI 3
  backend, views are WinUI 3 XAML controls by default.
* Native views: `View` (a container with Absolute, Row and Column layout),
  `Label`, `Button`, `TextField` and `ImageView`, plus `Window.contentView` to
  put them in a window. A control is a `View`, so it can be passed wherever a
  `View` is expected. Events: `ButtonClickedEvent`, `TextFieldChangedEvent`,
  `TextFieldSubmittedEvent`, `ViewFocusedEvent`, `ViewBlurredEvent`.
* `EdgeInsets` value type (`EdgeInsets.all`, `EdgeInsets.symmetric`).
* **Breaking:** event classes keep their C++ names. `KeyboardKeyPressedEvent`,
  `KeyboardKeyReleasedEvent` and `KeyboardModifierKeysChangedEvent` are now
  `KeyPressedEvent`, `KeyReleasedEvent` and `ModifierKeysChangedEvent`.
* **Breaking:** nativeapi is a plain Dart package and no longer depends on
  Flutter; Flutter apps depend on `nativeapi_flutter` instead, which re-exports
  it.
  * `Point`, `Size`, `Rectangle` and `Color` are nativeapi's own value types
    instead of `dart:ui`'s `Offset`, `Size`, `Rect` and `Color`.
    `nativeapi_flutter` converts between them (`toOffset()`, `toSize()`,
    `toRect()`, `toColor()`, and `toNative()` on the `dart:ui` types).
  * The widgets (`DragToMoveArea`, `DragToResizeArea`, `DragOutArea`,
    `DropRegion`, `ContextMenuRegion`), `ImageAsset` and
    `package:nativeapi/windowing.dart` moved to `nativeapi_flutter`
    (`package:nativeapi_flutter/windowing.dart`).
* Value types (`Point`, `Size`, `Rectangle`, `Color`, `KeyboardAccelerator`,
  `UrlOpenResult`, `ShortcutOptions`) compare and print by value: they have `==`,
  `hashCode` and `toString` over their data fields. Callback fields are left out.
* Callbacks are released once the core lets them go, instead of being kept
  for the life of the isolate: a listener when it is removed or its emitter
  destroyed, a replaced `setCallback` / `setWill{Show,Hide}Hook` callback, a
  shortcut's callback when it is unregistered or the shortcut destroyed, and
  any callback whose registration failed. A listener still lives until then,
  whether or not the wrapper that added it is collected.
* `ShortcutOptions.callback` is installed by `registerWithOptions` and
  `Shortcut.createWithIdAndOptions`; it used to be dropped.
* `WindowShadow`, `Window.setCustomShadow` and `customShadow`: core-rendered
  custom shadows on desktop windows with a hidden title bar. `hasShadow` toggles
  the shadow without discarding its configuration.
* `WindowShape`, `Window.setShape`, `isShaped` and `isShapeSupported` for
  polygonal desktop windows; the new `flutter_shaped_window_example` shows
  circle, star and speech-bubble shapes.
* `Window.setInputShape`, `isInputShaped` and `isInputShapeSupported` restrict
  pointer and touch input to a region on Linux (X11 and Wayland).
* **Breaking:** `TitleBarStyle.hidden` now means the same thing on every platform — no
  title bar and no window control buttons. On macOS it used to leave the traffic lights
  on a transparent bar, which Windows and Linux never did. A window that wants them back
  sets `isWindowControlButtonsVisible = true` after the style, and one that wanted the
  transparent bar all along wants the new call below.
* `Window.setContentUnderTitleBar(bool)` takes the title bar into the content area:
  it becomes a transparent overlay, the window buttons stay on it, and a background
  colour or visual effect runs unbroken to the top edge. It returns whether the platform
  can do it, and `Window.isContentUnderTitleBarSupported()` answers beforehand —
  macOS only. On Windows the client area can be given the caption band, but the caption
  buttons stop hit-testing once it is client area; on Linux a GTK header bar is a sibling
  above the content, not an overlay over it.
* `Window.titleBarStyle` no longer forces the window control buttons visible on macOS as
  a side effect; it sets them to what the style implies, which a following
  `isWindowControlButtonsVisible` overrides.
* **Breaking:** the `Window.visualEffect` setter is now `bool setVisualEffect(effect)`: it
  returns whether the effect is in force, and `Window.isVisualEffectSupported(effect)`
  answers beforehand. The getter stays, and now reports the effect the native window
  really has - `none` on Linux, where there are no visual effects, and after a refused
  call - instead of the last value that was set.
* A visual effect shows in a Flutter window. On macOS the effect view used to end up
  either over the Flutter content or behind its opaque black backing; the backing is now
  clear while an effect is active and comes back with `VisualEffect.none`. The window's
  `backgroundColor` is kept (and no longer reset on macOS) but not shown meanwhile. What
  Flutter paints is the app's business: use a transparent scaffold, or no `MaterialApp`
  at all as the new `visual_effect_example` does.
* Windows: every effect now works on Windows 10 as well, where it is a blur-behind
  kind — before, all of them needed Windows 11 22H2 and did nothing at all below it.
  On Windows 11 22H2 and later every effect is a system backdrop, which covers the
  title bar too; a blur-behind reaches the client area only, so on Windows 10
  `blur` stops at the title bar.
* New effects `micaAlt` (Windows 11 Mica Alt), `hud`, `popover` and `menu` (macOS
  materials; Acrylic on Windows).
* Windows: `Window.backgroundColor` of a window created with `Window.create()` no longer
  changes every other such window with it.
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
* Add `package:nativeapi/windowing.dart`: `controller.nativeWindow` / `nativeWindowOf()`
  turn a window created with Flutter's experimental multi-window API into a nativeapi
  `Window`, so Flutter-rendered secondary windows can be styled, positioned and observed
  from Dart alone. It is a separate library because it imports Flutter's internal
  windowing libraries, whose names change between releases; it follows the stable channel.
  Stable has no `flutter config --enable-windowing`, so an app switches the API on itself
  with `isWindowingEnabled = true` before its binding starts. See
  `floating_toolbar_example`, `browser_tabs_example` and `detachable_window_example`.
* Windows: a `Window.backgroundColor` with alpha now really makes the window see-through —
  the alpha was ignored before; the compositor draws the color. (macOS since 0.2.7.)
* Linux: a `Window.backgroundColor` with alpha makes a Flutter window see-through there
  too (the Flutter view's black backing takes the color), and `Window.setParentWindow`
  now reaches a Wayland compositor when the child happens to be shown before its parent.
* Linux: `Window.titleBarStyle = hidden` on a window that is not shown yet no longer
  leaves its content 52 px short in both directions on Wayland (it now hides GTK's own
  header bar instead of un-decorating the window).
* Linux: `Window.hasShadow = false` works on windows with client-side decorations — every
  toplevel on Wayland, and windows with a header bar on X11. A window the window manager
  decorates keeps its shadow, and `hasShadow` keeps saying so. `contentSize` of a window
  created by this library is no longer measured with the shadow margin and the title bar
  included on Wayland.
* Linux: building no longer asks for `libayatana-appindicator3-dev` (see cnativeapi).

## 0.2.7

* Add `WindowCreatedEvent` and `WindowClosedEvent`. `WindowManager.addListener` now reports
  a window the first time it is shown and when it is closed for good, whoever created or
  closed it — including windows made with Flutter's multi-window API. A window that is
  never shown emits neither; hiding is not closing.
* Add `Window.setParentWindow` / `Window.parentWindow` for child windows — floating
  toolbars, palettes, inspectors. A child stays above its parent and hides when the parent
  is minimized. On macOS it also moves with its parent; on Windows and Linux follow the
  parent's `WindowMovedEvent` for that. On Windows a child is destroyed with its parent.
* `WindowEvent` and the other event base classes now expose what every variant carries
  (`event.windowId`, …), so reading it no longer needs a match on the concrete type.
* macOS: a `Window.backgroundColor` with alpha now really makes the window see-through —
  the window is marked non-opaque, and a Flutter view in it drops its black backing.
* Add `floating_toolbar_example`: a transparent, frameless Flutter window attached to the
  main window, from Dart alone.
* Add `TrayIcon.isIconTemplate`, `iconSize` and `iconPosition` (with `TrayIconPosition`).
  **Behaviour change on macOS:** an icon is no longer forced to be a template image, so
  coloured icons keep their colours — set `isIconTemplate = true` for a monochrome glyph
  that should follow the menu bar. The default size is still 18 x 18 points; a zero size
  draws the image at its own size. The `Image` given to `icon` is no longer modified.
  Windows and Linux record the three values without using them.
* Windows: tray icons come back after Explorer restarts instead of disappearing for good.
* Linux: every tray icon gets its own D-Bus connection, so a second icon in the same
  process appears instead of failing to register, and a disposed icon leaves the panel
  at once.
* Linux: window size and position are measured by the frame and the content instead of
  the `GdkWindow`.
* Windows: a menu item's click event is emitted before `Menu.open` returns, and `open`
  reports failure from whether the menu was actually shown.
* Fix a crash when a window message arrived for a menu that had already been destroyed.

## 0.2.5

* Add drag and drop: `DropTarget` / `DragSource` bindings, and the `DropRegion` and
  `DragOutArea` widgets for dropping files and text onto a window and dragging them out
  of it.
* Add `DragToMoveArea` and `DragToResizeArea` widgets for custom window chrome, with
  optional target windows and configurable resize handles.
* Add `WindowDragSession` and `WindowManager.getWindowAtPoint` for tear-off windows that
  follow the cursor and can dock back into another window.
* Add `Window.isVisibleInTaskbar` for keeping a window out of the taskbar — the
  `skipTaskbar` counterpart, inverted so it reads like the other flags. Windows adds and
  removes the taskbar button (and keeps it away across hide/show), Linux sets the window
  manager's skip-taskbar hint, macOS drops the window from the app's Window menu.
* Window events now actually fire. `WindowResizedEvent`, `WindowMovedEvent`,
  `WindowMinimizedEvent`, `WindowMaximizedEvent` and `WindowRestoredEvent` existed all
  the way through the bindings but were never emitted — only focused and blurred worked.
  Wayland cannot report moves or minimization to a client, so those stay silent there.
* Fix `Window.hasShadow` doing nothing on Windows: turning it off now removes the
  window's drop shadow, which is what a frameless window usually wants.
* Fix `Window.titleBarStyle` on Windows: hiding the title bar now takes effect straight
  away instead of only after a fullscreen round trip, the content fills the space the
  title bar left, and the top edge stays resizable.
* Fix `Window.isFullScreen` on Windows being answered from the window's rectangle, so
  moving or centering a full screen window left it stuck in full screen for good. Also
  fix `Window.isMinimized` always being false there, which made `restore()` a no-op.
* Fix `Window.startDragging` not moving the window on Windows, and on macOS the gesture
  after a `startDragging` / `startResizing` being ignored; implement it on Linux.
* Linux fixes: `WindowManager.getCurrent()` and `Window.isFocused` now find the focused
  window, an app no longer exits immediately instead of running, opening a context menu
  relative to a window no longer crashes, and the display getters survive monitors being
  replaced or the screen blanking.
* Windows fixes: window hit testing and late drag starts, submenu detach/reattach, and
  Top End menu placement.
* Add `window_drag_areas_example`, `drag_drop_example`, `detachable_window_example` and
  `browser_tabs_example`.

## 0.2.4

* Update to `cnativeapi` 0.2.4 with Windows DLL export and Apple build fixes.
* Fix storage example initialization and collection access by using the `all` getter.
* Sync generated Dart bindings with the updated core.

## 0.2.3

* Add `Window.isNonActivating`. A non-activating window can be shown on top and take
  keyboard input without activating the app, so the previously active app keeps its
  activation and the app's other windows stay put when it is hidden. Only macOS has
  observable behavior (the window becomes a non-activating `NSPanel` in place); Windows
  and Linux record the flag
* macOS: `Window.isFocusable = false` now actually prevents the window from becoming key

## 0.2.2

* Fix macOS build failure in 0.2.1 coming from `cnativeapi` — see its changelog

## 0.2.1

* Add `AppInfo` API for the running app's name, identifier, version and build number
* Add `DeviceInfo` API for the machine's name, model, manufacturer, OS, kernel and architecture
* Fix `WindowFocusedEvent` and `WindowBlurredEvent` never firing — the native layer
  dispatched no window events at all, so `WindowManager.addListener` was silent

## 0.2.0

* **Breaking:** regenerate the Dart API from the new code generator; the object identity/lifecycle model is unified and many signatures changed
* **Breaking:** rework the event system — `EventEmitter` and the separate `WindowEvent`/`MenuEvent`/`TrayIconEvent` classes are replaced by typed event listeners on each object
* Add `Application` API
* Add `KeyboardMonitor` API
* Add `Shortcut` and `ShortcutManager` APIs for global shortcuts
* Add `ImageAsset` widget helper
* Fix tray icon bounds on multi-display setups (macOS)
* Fix macOS global shortcuts never firing

## 0.1.4

* Fix macOS menu item disabled state not being respected
* Fix Windows window `SetMinimumSize`/`SetMaximumSize` not working
* Add per-monitor DPI scaling for Windows display and window geometry
* Track menu item enabled state and update flags on Windows
* Adjust Linux tray icon menu trigger handling
* Enhance window example UI with event logging and three-tab interface
* Add menu example enhancements (disabled menu items, bug repro)
* Remove CocoaPods artifacts from macOS Xcode project

## 0.1.3

* Remove CocoaPods integration from the storage example
* Update Darwin example integration to use Swift Package Manager

## 0.1.2

* Add `LaunchAtLogin` API for managing app startup at user login
* Fix bindgen Dart template generation for `LaunchAtLogin` string-array arguments
* Fix bindgen Dart template generation for static native API calls

## 0.1.1

* Add `WindowManager` API with full lifecycle support (show, hide, center, pre-show/pre-hide hooks, visual effects, title bar style, control buttons)
* Add `TrayManager` / `TrayIcon` API with context menu trigger support
* Add `DisplayManager` / `Display` multi-screen management API
* Add `Menu` / `MenuEvent` menu system
* Add `MessageDialog` / `Dialog` APIs
* Add `AccessibilityManager` API
* Add `Preferences` storage
* Add `SecureStorage` secure storage
* Add `UrlOpener` API
* Add `PositioningStrategy` / `Placement` window positioning support
* Add `ContextMenuRegion` widget

## 0.1.0

* Initial release
