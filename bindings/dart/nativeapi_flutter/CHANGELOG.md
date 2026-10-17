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

* Requires nativeapi 0.5.2.

## 0.5.1

* Requires nativeapi 0.5.1, which no longer leaves objects behind or crashes
  on a hot restart.

## 0.5.0

* Requires nativeapi 0.5.0.

* `DragToMoveArea` opens the native system window menu on a secondary click:
  on the release on Windows, as a native title bar does, and on the press on
  supporting Linux window managers, which a Wayland compositor requires.

* `MaximizeButtonArea`: wrap a custom title bar's maximize button in it and
  Windows 11 shows the snap layouts when the pointer rests on it. The button
  still gets hover, press and tap; the area follows it and goes away with it.

* `package:nativeapi_flutter/windowing.dart` compiles for the web too
  (leanflutter/tray_manager#108): there `nativeWindowOf` and `nativeWindow`
  return null, since no native window stands behind a controller.

* `DragToMoveArea` follows the macOS title-bar double-click preference,
  including Minimize and None, through the native window action.

## 0.4.1

* Requires nativeapi 0.4.1.
* `runNativeApp` is not re-exported: it is for plain Dart programs, and a
  Flutter app's engine already runs the event loop.

## 0.4.0

* Initial release. It re-exports `package:nativeapi`.
* `View`, `TextField` and `EdgeInsets` from nativeapi are not re-exported, as
  Flutter already has widgets and a class of those names; use the native ones
  through a prefixed `package:nativeapi/nativeapi.dart` import.
* Holds the Flutter side of nativeapi, which is now a plain Dart package: the
  widgets (`DragToMoveArea`, `DragToResizeArea`, `DragOutArea`, `DropRegion`,
  `ContextMenuRegion`), `ImageAsset` and `package:nativeapi_flutter/windowing.dart`
  moved here from `nativeapi`.
* Conversions between nativeapi's `Point`, `Size`, `Rectangle`, `Color` and
  `Brightness` and `dart:ui`'s `Offset`, `Size`, `Rect`, `Color` and
  `Brightness`.
* The re-export of `package:nativeapi` leaves out `Brightness`, `Color`,
  `Display`, `Image`, `ModifierKey`, `ShortcutManager` and `Size`, which clash
  with Flutter's names.
