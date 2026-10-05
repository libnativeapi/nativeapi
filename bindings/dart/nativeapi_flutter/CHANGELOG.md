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
