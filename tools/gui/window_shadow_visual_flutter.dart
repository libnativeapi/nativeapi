import 'dart:ffi' hide Size;
import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:window_manager/window_manager.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final inspectNative = DynamicLibrary.executable()
      .lookupFunction<Int32 Function(Int32), int Function(int)>(
        'shadow_test_inspect',
      );
  final window = WindowManager.instance.getCurrent()!;
  window.titleBarStyle = TitleBarStyle.hidden;
  window.setSize(const Size(800, 600).toNative(), false);
  window.title = 'NativeAPI shadow visual regression';
  runApp(
    const Directionality(
      textDirection: TextDirection.ltr,
      child: ColoredBox(
        color: Color(0xFF244865),
        child: Center(child: Text('Shadow visual regression')),
      ),
    ),
  );
  window.show();
  Future<void> settle() => Future<void>.delayed(const Duration(seconds: 1));
  int inspect(int fitted) {
    final view = WidgetsBinding.instance.platformDispatcher.views.single;
    final logical = view.physicalSize / view.devicePixelRatio;
    final content = window.contentSize.toSize();
    final ok =
        (logical.width - content.width).abs() <= 1 &&
        (logical.height - content.height).abs() <= 1;
    stdout.writeln(
      '${ok ? "PASS" : "FAIL"} Flutter view=$logical native content=$content',
    );
    return inspectNative(fitted) + (ok ? 0 : 1);
  }

  await settle();
  window.setSize(const Size(800, 600).toNative(), false);
  await settle();
  var failures = inspect(0);
  for (final shadow in [false, true]) {
    window.hasShadow = shadow;
    await settle();
    failures += inspect(0);
  }
  for (var cycle = 0; cycle < 2; cycle++) {
    window.maximize();
    await settle();
    failures += inspect(1);
    window.unmaximize();
    await settle();
    failures += inspect(0);
  }
  window.hide();
  window.show();
  await settle();
  failures += inspect(0);
  stdout.writeln('RESULT failures=$failures');
  window.close();
}
