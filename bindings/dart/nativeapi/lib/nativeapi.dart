library;

// Every generated module. The list itself is generated too, so adding a header
// upstream does not need an edit here.
//
// `dart:ffi` does not exist on the web, and importing it fails the whole
// build. There the same API comes from a pure-Dart mirror whose platform
// calls throw `UnsupportedError`, so an app that also targets the web can
// still depend on nativeapi and guard its calls with `kIsWeb`.
export 'src/generated.dart'
    if (dart.library.js_interop) 'src/web/generated.dart';

// Hand-written.
export 'src/run_native_app.dart'
    if (dart.library.js_interop) 'src/run_native_app_web.dart';
