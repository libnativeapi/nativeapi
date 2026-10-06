// ignore_for_file: invalid_use_of_internal_member, implementation_imports

import 'package:flutter/src/widgets/_window.dart' as fw;

import 'package:nativeapi/nativeapi.dart' show Window;

/// The native window behind a Flutter window controller, as a nativeapi
/// [Window].
///
/// Always null on the web: there is no native window behind a controller, and
/// the platform implementations (and `dart:ffi`) do not exist there.
Window? nativeWindowOf(fw.BaseWindowController controller) => null;

extension FlutterWindowControllerNativeWindow on fw.BaseWindowController {
  /// Shorthand for [nativeWindowOf].
  Window? get nativeWindow => nativeWindowOf(this);
}
