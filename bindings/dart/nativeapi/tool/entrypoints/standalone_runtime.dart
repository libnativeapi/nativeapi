// No input or visible window: verify UI-thread creation, a Dart timer and quit.
import 'dart:async';

import 'package:nativeapi/standalone.dart';

void appMain() {
  final window = Window.create();
  if (window == null) throw StateError('Could not create a native window');
  window.title = 'Standalone entrypoint smoke';
  Timer(const Duration(milliseconds: 100), () {
    if (window.title != 'Standalone entrypoint smoke') {
      throw StateError('Native window did not retain its title');
    }
    print('PASS standalone callback, native window, Dart timer and quit');
    Application.instance.quit(0);
  });
}

void main() => runNativeApp(appMain);
