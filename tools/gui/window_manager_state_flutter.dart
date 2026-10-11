// No-input regression for the nativeapi migration. Built as an alternative
// entry point of window_manager's example by flutter_window_manager_state_test.py.
// ignore_for_file: deprecated_member_use

import 'dart:async';
import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:nativeapi/nativeapi.dart' as native;
import 'package:window_manager/legacy.dart';

class Reports with WindowListener {
  final events = <String>[];
  void Function()? closeAction;

  @override
  void onWindowEvent(String eventName) => events.add(eventName);

  @override
  void onWindowClose() => closeAction?.call();
}

Future<void> until(bool Function() ready, String label) async {
  final deadline = DateTime.now().add(const Duration(seconds: 12));
  while (!ready()) {
    if (DateTime.now().isAfter(deadline)) throw StateError(label);
    await Future<void>.delayed(const Duration(milliseconds: 40));
  }
}

void check(bool ok, String label) {
  if (!ok) throw StateError(label);
  stdout.writeln('PASS $label');
}

Future<void> tests() async {
  final window = native.WindowManager.instance.getCurrent()!;
  final reports = Reports();
  windowManager.addListener(reports);
  try {
    await windowManager.setTitle('nativeapi migration regression');
    check(
      window.title == 'nativeapi migration regression',
      'legacy title reaches core',
    );
    await windowManager.setSize(const Size(640, 480));
    await until(() => (window.size.width - 640).abs() < 2, 'size settles');
    check(
      (await windowManager.getSize()).width == window.size.width,
      'legacy geometry reads the native window',
    );

    // Leave the compatibility layer unsubscribed while the window changes.
    windowManager.removeListener(reports);
    window.maximize();
    await until(() => window.isMaximized, 'maximize settles');
    await Future<void>.delayed(const Duration(milliseconds: 500));
    windowManager.addListener(reports);
    reports.events.clear();
    await windowManager.unmaximize();
    await until(
      () => reports.events.contains('unmaximize'),
      'unmaximize after re-subscribe',
    );
    check(
      !reports.events.contains('restore'),
      'late listener distinguishes unmaximize from restore',
    );

    windowManager.removeListener(reports);
    window.isFullScreen = true;
    await until(() => window.isFullScreen, 'full screen settles');
    // macOS finishes its transition asynchronously after setting the flag.
    await Future<void>.delayed(const Duration(seconds: 2));
    windowManager.addListener(reports);
    reports.events.clear();
    await windowManager.setFullScreen(false);
    await until(
      () => reports.events.contains('leave-full-screen'),
      'exit full screen after re-subscribe',
    );
    check(true, 'late listener reports leaving full screen');
    await Future<void>.delayed(const Duration(seconds: 2));

    // Another native listener holds the window open, independently of legacy.
    // This lets us verify ordinary close notifications without ending the app.
    var requests = 0;
    final veto = window.addListener((event) {
      if (event is native.WindowCloseRequestedEvent) {
        requests++;
        event.request.cancel();
      }
    });
    try {
      reports.events.clear();
      await windowManager.setPreventClose(false);
      await windowManager.close();
      await until(
        () => reports.events.contains('close'),
        'ordinary close notification',
      );
      check(true, 'close reports without preventClose');

      reports.events.clear();
      await windowManager.setPreventClose(true);
      await windowManager.destroy();
      await Future<void>.delayed(const Duration(milliseconds: 300));
      check(
        !reports.events.contains('close'),
        'destroy bypasses the legacy close callback',
      );

      // A vetoed destroy must not leave every future close bypassing legacy.
      await windowManager.close();
      await until(
        () => reports.events.contains('close'),
        'preventClose after vetoed destroy',
      );
      check(
        true,
        'preventClose survives a destroy vetoed by another native listener',
      );

      // A common 0.5.x pattern destroys the window from onWindowClose itself.
      // The native veto lets the fixture count the separate destroy request.
      await Future<void>.delayed(const Duration(milliseconds: 300));
      final before = requests;
      reports.closeAction = () => windowManager.destroy();
      await windowManager.close();
      await until(() => requests >= before + 2, 'destroy from onWindowClose');
      reports.closeAction = null;
      check(true, 'destroy from onWindowClose starts a separate close request');
    } finally {
      window.removeListener(veto);
    }
  } finally {
    windowManager.removeListener(reports);
  }
}

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await windowManager.ensureInitialized();
  await windowManager.waitUntilReadyToShow(
    const WindowOptions(
      size: Size(640, 480),
      title: 'nativeapi migration regression',
    ),
  );
  runApp(
    const Directionality(
      textDirection: TextDirection.ltr,
      child: ColoredBox(
        color: Color(0xFFF0F0F0),
        child: Center(child: Text('Native migration regression')),
      ),
    ),
  );
  WidgetsBinding.instance.addPostFrameCallback((_) async {
    try {
      await tests();
      stdout.writeln('PASS window_manager native state regression');
      native.Application.instance.quit(0);
    } catch (error, stack) {
      stderr.writeln('FAIL $error\n$stack');
      native.Application.instance.quit(1);
    }
  });
}
