/// Bridges Flutter's experimental multi-window API to nativeapi.
///
/// Kept out of `package:nativeapi/nativeapi.dart` because it imports Flutter's
/// internal windowing libraries, whose names still change between releases; it
/// follows the stable channel (checked with Flutter 3.47.5). Apps that never
/// import this library are unaffected by changes to those internals.
///
/// The stable channel does not offer `flutter config --enable-windowing`: an app
/// sets `isWindowingEnabled = true` (from
/// `package:flutter/src/foundation/_features.dart`) before its binding starts.
///
/// On the web, where there are no native windows, [nativeWindowOf] returns
/// null, so an app that also targets the web can still import this library.
library;

export 'src/windowing/flutter_window.dart'
    if (dart.library.js_interop) 'src/windowing/flutter_window_web.dart';
export 'src/windowing/window_geometry.dart';
