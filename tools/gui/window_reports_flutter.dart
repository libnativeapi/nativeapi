// Fixture for tools/gui/flutter_window_reports_test.ps1: the window_manager
// reports carried over to nativeapi-core (#75-#78), replayed through the real
// nativeapi Flutter Window API in a real Flutter Windows app.
//
//   window_reports.exe <scenario> <handshake-dir>
//
// The app paints solid colours the runner can tell apart in screenshots: a
// green background (also the native window's background colour), an amber
// panel in the middle and a blue bar along the bottom. A scenario is a list of
// steps. Before each one the app prints `STEP <name>` with the window's state
// and waits for `<handshake-dir>/go-<name>`, so the runner can start a frame
// capture or measure first; after the last one it prints `DONE`.
import 'dart:async';
import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

const background = Color(0xFF2E7D32);
const panel = Color(0xFFFFC107);
const bar = Color(0xFF1565C0);

late final String handshake;
late final Window window;
final events = <String>[];

Future<void> main(List<String> args) async {
  WidgetsFlutterBinding.ensureInitialized();
  final scenario = args.isNotEmpty ? args[0] : 'fullscreen';
  handshake = args.length > 1 ? args[1] : Directory.systemTemp.path;
  final current = WindowManager.instance.getCurrent();
  if (current == null) {
    print('FAIL no current window');
    exit(1);
  }
  window = current;
  window.backgroundColor = background.toNative();
  WindowManager.instance.addListener((event) {
    if (event.windowId != window.id) return;
    final name = event.runtimeType
        .toString()
        .replaceAll('Window', '')
        .replaceAll('Event', '');
    events.add(name);
    print('EVENT $name');
  });
  final run = scenarios[scenario];
  if (run == null) {
    print('FAIL unknown scenario $scenario');
    exit(1);
  }
  await run.before?.call();
  runApp(const ReportsApp());
  await WidgetsBinding.instance.endOfFrame;
  await pause(1500);
  for (final step in run.steps) {
    await announce(step.name);
    await step.action();
    await pause(step.settle);
  }
  await announce('end');
  print('DONE');
  exit(0);
}

class Scenario {
  const Scenario(this.steps, {this.before});
  final Future<void> Function()? before;
  final List<Step> steps;
}

class Step {
  const Step(this.name, this.action, {this.settle = 1200});
  final String name;
  final FutureOr<void> Function() action;
  final int settle;
}

Future<void> pause(int ms) => Future.delayed(Duration(milliseconds: ms));

String state() {
  final b = window.bounds;
  return 'bounds=${b.x.round()},${b.y.round()},${b.width.round()},${b.height.round()} '
      'fullScreen=${window.isFullScreen} maximized=${window.isMaximized} '
      'minimized=${window.isMinimized} visible=${window.isVisible} '
      'focused=${window.isFocused} titleBar=${window.titleBarStyle.name}';
}

Future<void> announce(String name) async {
  print('STEP $name ${state()}');
  final go = File('$handshake${Platform.pathSeparator}go-$name');
  final deadline = DateTime.now().add(const Duration(seconds: 60));
  while (!go.existsSync()) {
    if (DateTime.now().isAfter(deadline)) {
      print('FAIL no go for $name');
      exit(1);
    }
    await pause(20);
  }
}

/// A path inside flutter_assets, as window_manager#534 used for setIcon().
String flutterAsset(String name) {
  final dir = File(Platform.resolvedExecutable).parent.path;
  final sep = Platform.pathSeparator;
  return '$dir${sep}data${sep}flutter_assets$sep${name.replaceAll('/', sep)}';
}

final scenarios = <String, Scenario>{
  // #77: leanflutter/window_manager#458, #579, #330.
  'fullscreen': Scenario(
    before: () async =>
        window.isFullScreen = true, // Full screen before the first frame.
    [
      Step('leave-fullscreen', () => window.isFullScreen = false),
      Step('minimize', () => window.minimize()),
      Step('restore', () => window.restore()),
      Step('enter-fullscreen', () => window.isFullScreen = true, settle: 1500),
      Step(
        'leave-fullscreen-again',
        () => window.isFullScreen = false,
        settle: 1500,
      ),
    ],
  ),
  // #76: leanflutter/window_manager#554, #450, #378, #397, #547.
  'hidden': Scenario(
    before: () async => window.titleBarStyle = TitleBarStyle.hidden,
    [
      Step('maximize', () => window.maximize()),
      Step('restore', () => window.unmaximize()),
      Step('enter-fullscreen', () => window.isFullScreen = true, settle: 1500),
      Step('leave-fullscreen', () => window.isFullScreen = false, settle: 1500),
    ],
  ),
  // #76: leanflutter/window_manager#576.
  'transparent': Scenario(
    before: () async =>
        window.backgroundColor = const Color(0x00000000).toNative(),
    [Step('look', () {})],
  ),
  // #78: leanflutter/window_manager#294, #207, #511, #534. The runner acts
  // (double-clicks) before letting a step go; its action then only settles.
  'events': Scenario([
    Step('minimize', () => window.minimize()),
    Step('restore', () => window.restore()),
    Step('double-click-caption', () {}, settle: 1500),
    Step('double-click-caption-again', () {}, settle: 1500),
    Step('double-tap-panel', () {}, settle: 1500),
    Step('restore-after-tap', () => window.unmaximize()),
    Step('set-icon', () {
      final ok = Application.instance.setIcon(flutterAsset('assets/icon.ico'));
      print('ICON setIcon=$ok');
    }),
  ]),
};

class ReportsApp extends StatelessWidget {
  const ReportsApp({super.key});

  @override
  Widget build(BuildContext context) {
    return Directionality(
      textDirection: TextDirection.ltr,
      child: ColoredBox(
        color: background,
        child: Column(
          children: [
            Expanded(
              child: Center(
                // window_manager#511: maximize() from onDoubleTapDown, while
                // the pointer is still down (and captured).
                child: GestureDetector(
                  onDoubleTapDown: (_) {
                    print('TAP double-down');
                    window.maximize();
                  },
                  child: Container(
                    width: 240,
                    height: 140,
                    color: panel,
                    alignment: Alignment.center,
                    child: const Text(
                      'nativeapi reports',
                      style: TextStyle(color: Color(0xFF000000), fontSize: 18),
                    ),
                  ),
                ),
              ),
            ),
            Container(height: 48, color: bar),
          ],
        ),
      ),
    );
  }
}
