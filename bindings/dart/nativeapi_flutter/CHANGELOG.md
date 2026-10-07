## 0.4.1

* `MaximizeButtonArea`: wrap a custom title bar's maximize button in it and
  Windows 11 shows the snap layouts when the pointer rests on it. The button
  still gets hover, press and tap; the area follows it and goes away with it.

* `package:nativeapi_flutter/windowing.dart` compiles for the web too
  (leanflutter/tray_manager#108): there `nativeWindowOf` and `nativeWindow`
  return null, since no native window stands behind a controller.

* `DragToMoveArea` follows the macOS title-bar double-click preference,
  including Minimize and None, through the native window action.

* Requires nativeapi 0.4.1.
* `DragToMoveArea` opens the native system window menu on a secondary-button
  press on Windows and supporting Linux window managers.
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
