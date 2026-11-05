/// Native system APIs and the experimental event loop for plain Dart apps.
///
/// Re-exports `package:nativeapi/nativeapi.dart` and adds `runNativeApp`.
/// The runner requires experimental VM APIs available in Dart 3.13 and is not
/// supported on Dart 3.14, where those APIs were removed.
///
/// Use `package:nativeapi/nativeapi.dart` for the ordinary API, or
/// `package:nativeapi_flutter/nativeapi_flutter.dart` in Flutter apps. Those
/// entrypoints do not import the experimental runner.
library;

export 'nativeapi.dart';
export 'src/run_native_app.dart'
    if (dart.library.js_interop) 'src/run_native_app_web.dart';
