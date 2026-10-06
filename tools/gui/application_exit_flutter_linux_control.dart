// ignore_for_file: invalid_use_of_internal_member, implementation_imports
// Flutter-only control: no nativeapi imports or native assets.
import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:ui' show PlatformDispatcher;

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' as fw;
import 'package:flutter/src/widgets/_window_linux.dart' as fl;

@ffi.Native<ffi.Void Function(ffi.Pointer<ffi.Void>)>(symbol: 'gtk_widget_hide')
external void gtkHide(ffi.Pointer<ffi.Void> widget);
@ffi.Native<ffi.Void Function(ffi.Pointer<ffi.Void>)>(symbol: 'gtk_widget_show')
external void gtkShow(ffi.Pointer<ffi.Void> widget);
@ffi.Native<ffi.Void Function(ffi.Pointer<ffi.Void>)>(
  symbol: 'gtk_window_close',
)
external void gtkClose(ffi.Pointer<ffi.Void> window);

Future<void> frames() async {
  for (var i = 0; i < 3; i++) {
    WidgetsBinding.instance.scheduleFrame();
    await WidgetsBinding.instance.endOfFrame;
    await Future<void>.delayed(const Duration(milliseconds: 80));
  }
}

class Host with fw.RegularWindowControllerDelegate {
  int destroyed = 0;
  @override
  void onWindowDestroyed() => destroyed++;
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

Future<void> tests() async {
  runWidget(scene());
  await frames();
  final implicitId = PlatformDispatcher.instance.implicitView!.viewId;
  for (final nativeClose in [false, true]) {
    final host = Host();
    final controller = fw.RegularWindowController(
      size: const Size(320, 220),
      delegate: host,
    );
    if (controller.rootView.viewId == implicitId)
      throw StateError('implicit view reused');
    runWidget(scene(controller));
    await frames();
    final handle = (controller as fl.WindowControllerLinux).windowHandle;
    gtkHide(handle);
    gtkShow(handle);
    await frames();
    runWidget(scene());
    await frames();
    if (nativeClose) {
      gtkClose(handle);
    } else {
      controller.destroy();
    }
    await frames();
    if (host.destroyed != 1 ||
        PlatformDispatcher.instance.views.length != 1 ||
        PlatformDispatcher.instance.views.single.viewId != implicitId) {
      throw StateError('Flutter-only secondary teardown failed');
    }
    print('CONTROL_PASS: secondary view destroyed and implicit view retained');
  }
  print('FLUTTER_LINUX_CONTROL_EXIT_READY');
  await SystemNavigator.pop();
}

void main() {
  isWindowingEnabled = true;
  WidgetsFlutterBinding.ensureInitialized();
  FlutterError.onError = (details) {
    print('FLUTTER_LINUX_EXIT_FAIL: ${details.exceptionAsString()}');
    FlutterError.dumpErrorToConsole(details);
  };
  unawaited(
    tests().catchError((Object error, StackTrace stack) {
      print('FLUTTER_LINUX_EXIT_FAIL: $error\n$stack');
      SystemNavigator.pop();
    }),
  );
}
