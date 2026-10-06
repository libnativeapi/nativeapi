// ignore_for_file: invalid_use_of_internal_member, implementation_imports
// Runs in a real GTK Flutter runner on a private X11/Wayland display.
import 'dart:async';
import 'dart:ui' show PlatformDispatcher;

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' as fw;
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/windowing.dart' as bridge;

void check(bool condition, String label) {
  if (!condition) throw StateError(label);
  print('PASS: $label');
}

Future<void> frames() async {
  for (var i = 0; i < 3; i++) {
    WidgetsBinding.instance.scheduleFrame();
    await WidgetsBinding.instance.endOfFrame;
    await Future<void>.delayed(const Duration(milliseconds: 80));
  }
}

class WindowHost with fw.RegularWindowControllerDelegate {
  int requested = 0, destroyed = 0;

  @override
  void onWindowCloseRequested(fw.RegularWindowController controller) {
    requested++;
    controller.destroy();
  }

  @override
  void onWindowDestroyed() {
    destroyed++;
  }
}

Widget scene([fw.RegularWindowController? controller]) => ViewCollection(
  views: [
    View(
      view: PlatformDispatcher.instance.implicitView!,
      child: const ColoredBox(color: Color(0xff234567)),
    ),
    if (controller != null)
      fw.RegularWindow(
        controller: controller,
        child: const ColoredBox(color: Color(0xffabcdef)),
      ),
  ],
);

Future<void> tests({required bool leaveManagerLive}) async {
  final manager = na.WindowManager.instance;
  runWidget(scene());
  await frames();
  final listener = leaveManagerLive ? null : manager.addListener((_) {});
  var shown = 0, hidden = 0;
  if (leaveManagerLive) {
    final windows = manager.getAll();
    check(windows.isNotEmpty, 'native manager tracks the live GTK runner');
    for (final window in windows) {
      window.dispose();
    }
  } else {
    manager.setWillShowHook((_) => shown++);
    manager.setWillHideHook((_) => hidden++);
    check(
      manager.hasWillShowHook() && manager.hasWillHideHook(),
      'nativeapi global GTK hooks are installed',
    );
  }
  final implicitId = PlatformDispatcher.instance.implicitView!.viewId;

  for (final throughNative in [false, true]) {
    final host = WindowHost();
    final shownBefore = shown, hiddenBefore = hidden;
    final controller = fw.RegularWindowController(
      size: const Size(320, 220),
      title: 'nativeapi Linux exit regression',
      delegate: host,
    );
    final secondaryId = controller.rootView.viewId;
    final window = bridge.nativeWindowOf(controller)!;
    check(
      secondaryId != implicitId,
      'secondary controller has a distinct real Flutter view',
    );
    runWidget(scene(controller));
    await frames();
    check(
      PlatformDispatcher.instance.views.length == 2 &&
          WidgetsBinding.instance.renderViews.length == 2,
      'implicit and secondary views are registered and rendered',
    );
    check(
      leaveManagerLive ? window.isVisible : shown > shownBefore,
      'the realized secondary GTK window is shown',
    );
    window.hide();
    final becameHidden = !window.isVisible;
    window.show();
    await frames();
    check(
      leaveManagerLive ? becameHidden : hidden > hiddenBefore,
      'the realized secondary GTK window is hidden',
    );

    // Detach the widget before destroying its native view, as the application
    // owns both the widget tree and the controller lifetime.
    runWidget(scene());
    await frames();
    if (throughNative) {
      check(window.close(), 'nativeapi Window.close reaches the Flutter host');
    } else {
      controller.destroy();
    }
    await frames();
    check(
      host.destroyed == 1 && host.requested == (throughNative ? 1 : 0),
      'Flutter owns secondary teardown exactly once',
    );
    check(
      PlatformDispatcher.instance.views.length == 1 &&
          PlatformDispatcher.instance.views.single.viewId == implicitId,
      'secondary teardown preserves the implicit engine view',
    );
    check(
      bridge.nativeWindowOf(controller) == null,
      'the destroyed controller no longer exposes a native window',
    );
    check(
      WidgetsBinding.instance.renderViews.length == 1,
      'the secondary render tree is removed before engine view teardown',
    );
    window.dispose();
  }

  if (!leaveManagerLive) {
    manager.setWillShowHook(null);
    manager.setWillHideHook(null);
    manager.removeListener(listener!);
  } else {
    // No Dart callbacks are registered in this mode. Keep core's constructor-
    // installed GTK emission hooks active through the real embedder exit and
    // static manager destruction, rather than cleaning them up in the test.
    print('FLUTTER_LINUX_LIVE_MANAGER_EXIT_READY');
  }
  print('FLUTTER_LINUX_EXIT_READY');
  // Ask the actual embedder to quit; do not bypass GTK/engine teardown with
  // dart:io exit(), a process kill, or a fake platform channel.
  await SystemNavigator.pop();
}

void main(List<String> arguments) {
  isWindowingEnabled = true;
  WidgetsFlutterBinding.ensureInitialized();
  FlutterError.onError = (details) {
    print('FLUTTER_LINUX_EXIT_FAIL: ${details.exceptionAsString()}');
    FlutterError.dumpErrorToConsole(details);
  };
  unawaited(
    tests(leaveManagerLive: arguments.contains('--live-manager'))
        .catchError((Object error, StackTrace stack) {
          print('FLUTTER_LINUX_EXIT_FAIL: $error\n$stack');
          SystemNavigator.pop();
        }),
  );
}
