// Hand-written: `runNativeApp` where there is no platform event loop to run.

import 'web/unsupported.dart';

/// Runs a desktop app from a plain Dart program; unavailable on the web.
void runNativeApp(void Function() main) => unsupported();
