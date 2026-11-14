import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:window_manager/window_manager.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final window = WindowManager.instance.getCurrent()!;
  final minimum = Platform.environment['WM_SKIP_MINIMUM'] != '1';
  window.titleBarStyle = TitleBarStyle.hidden;
  if (minimum) window.minimumSize = const Size(800, 600).toNative();
  window.setSize(const Size(800, 600).toNative(), false);
  window.title = 'NativeAPI minimum size regression';
  runApp(
    const Directionality(
      textDirection: TextDirection.ltr,
      child: ColoredBox(
        color: Color(0xFF244865),
        child: Center(child: Text('800 x 600')),
      ),
    ),
  );
  window.show();
  await Future<void>.delayed(const Duration(seconds: 1));
  window.setSize(const Size(800, 600).toNative(), false);
  await Future<void>.delayed(const Duration(seconds: 1));
  var failures = 0;
  void checkSize(String phase) {
    final size = window.size.toSize();
    final ok = (size.width - 800).abs() <= 2 && (size.height - 600).abs() <= 2;
    stdout.writeln('${ok ? "PASS" : "FAIL"} minimum=$minimum $phase $size');
    if (!ok) failures++;
  }

  checkSize('initial');
  for (var cycle = 1; cycle <= 3; cycle++) {
    window.maximize();
    await Future<void>.delayed(const Duration(seconds: 1));
    stdout.writeln('SIZE maximized=${window.size.toSize()}');
    window.unmaximize();
    await Future<void>.delayed(const Duration(seconds: 1));
    checkSize('restored cycle=$cycle');
  }
  stdout.writeln('RESULT failures=$failures');
  window.close();
}
