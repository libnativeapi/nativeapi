// Real TextField and display fixture for core #76 / #78. No global shortcuts.
import 'dart:async';
import 'dart:convert';
import 'dart:developer' as developer;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:nativeapi/nativeapi.dart' as native;
import 'package:nativeapi_flutter/nativeapi_flutter.dart' show DragToMoveArea;

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  final window = native.WindowManager.instance.getCurrent()!;
  window.title = 'NativeAPI remaining Windows reports';
  window.contentSize = const native.Size(width: 640, height: 460);
  window.center();
  runApp(const ReportsApp());
}

class ReportsApp extends StatefulWidget {
  const ReportsApp({super.key});
  @override
  State<ReportsApp> createState() => _ReportsAppState();
}

class _ReportsAppState extends State<ReportsApp> {
  final focus = FocusNode();
  final keys = <String>[];
  final errors = <String>[];
  late final native.Window window;
  late final int listener;
  int submitted = 0;
  bool wasHidden = false;
  bool wasMinimized = false;

  @override
  void initState() {
    super.initState();
    window = native.WindowManager.instance.getCurrent()!;
    listener = window.addListener((event) {
      if (event is native.WindowFocusedEvent) focus.requestFocus();
    });
    HardwareKeyboard.instance.addHandler(key);
    final originalError = FlutterError.onError;
    FlutterError.onError = (details) {
      if (errors.isEmpty) print('FIRST FLUTTER ERROR: $details');
      errors.add(details.exceptionAsString());
      originalError?.call(details);
    };
    developer.registerExtension('ext.nativeapi.windowReport', (
      method,
      params,
    ) async {
      try {
        switch (params['action']) {
          case 'style':
            window.titleBarStyle = params['style'] == 'hidden'
                ? native.TitleBarStyle.hidden
                : native.TitleBarStyle.normal;
            window.contentSize = const native.Size(width: 640, height: 460);
          case 'prepare':
            keys.clear();
            submitted = 0;
            wasHidden = false;
            wasMinimized = false;
            focus.requestFocus();
          case 'cycle':
            if (params['kind'] == 'hide') {
              window.hide();
            } else {
              window.minimize();
            }
            await Future<void>.delayed(const Duration(milliseconds: 350));
            wasHidden = !window.isVisible;
            wasMinimized = window.isMinimized;
            window.show();
            window.focus();
          case 'move':
            final displays = native.DisplayManager.instance.getAll();
            try {
              final display = displays[int.parse(params['index']!)];
              final bounds = display.workArea;
              window.position = native.Point(
                x: bounds.x + 100,
                y: bounds.y + 100,
              );
            } finally {
              for (final display in displays) {
                display.dispose();
              }
            }
          case 'size':
            window.isResizable = params['resizable'] != 'false';
            window.contentSize = const native.Size(width: 320, height: 460);
          case 'state':
            break;
          default:
            return developer.ServiceExtensionResponse.error(
              developer.ServiceExtensionResponse.invalidParams,
              'Unknown action',
            );
        }
        await Future<void>.delayed(const Duration(milliseconds: 400));
        if (mounted) setState(() {});
        await WidgetsBinding.instance.endOfFrame;
        return developer.ServiceExtensionResponse.result(jsonEncode(state()));
      } catch (error, stack) {
        return developer.ServiceExtensionResponse.error(
          developer.ServiceExtensionResponse.extensionError,
          '$error\n$stack',
        );
      }
    });
  }

  bool key(KeyEvent event) {
    if (event.logicalKey == LogicalKeyboardKey.enter ||
        event.logicalKey == LogicalKeyboardKey.escape) {
      final name = event.logicalKey == LogicalKeyboardKey.enter
          ? 'enter'
          : 'escape';
      final direction = event is KeyDownEvent
          ? 'down'
          : event is KeyUpEvent
          ? 'up'
          : 'repeat';
      keys.add('$name:$direction');
      print('KEY ${keys.last}');
      if (mounted) setState(() {});
    }
    return false;
  }

  List<double> rectangle(native.Rectangle value) => [
    value.x,
    value.y,
    value.width,
    value.height,
  ];

  Map<String, Object?> state() {
    final view = View.of(context);
    final displays = native.DisplayManager.instance.getAll();
    try {
      return {
        'hwnd': window.nativeObject.address,
        'focused': window.isFocused,
        'textFocused': focus.hasFocus,
        'wasHidden': wasHidden,
        'wasMinimized': wasMinimized,
        'keys': keys,
        'submitted': submitted,
        'bounds': rectangle(window.bounds),
        'content': rectangle(window.contentBounds),
        'view': [
          view.physicalSize.width / view.devicePixelRatio,
          view.physicalSize.height / view.devicePixelRatio,
        ],
        'dpr': view.devicePixelRatio,
        'errors': errors,
        'displays': [
          for (final display in displays)
            {
              'id': display.id,
              'handle': display.nativeObject.address,
              'scale': display.scaleFactor,
              'bounds': [
                display.position.x,
                display.position.y,
                display.size.width,
                display.size.height,
              ],
              'work': rectangle(display.workArea),
            },
        ],
      };
    } finally {
      for (final display in displays) {
        display.dispose();
      }
    }
  }

  @override
  void dispose() {
    HardwareKeyboard.instance.removeHandler(key);
    window.removeListener(listener);
    focus.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => MaterialApp(
    home: Scaffold(
      backgroundColor: const Color(0xff2e7d32),
      body: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          children: [
            const SizedBox(height: 30),
            DragToMoveArea(
              window: window,
              child: const Text('Drag between displays'),
            ),
            TextField(
              autofocus: true,
              focusNode: focus,
              onSubmitted: (_) {
                submitted++;
                focus.requestFocus();
              },
              decoration: const InputDecoration(
                hintText: 'Enter and Escape verification',
              ),
            ),
            Text('Key events: ${keys.length}'),
          ],
        ),
      ),
    ),
  );
}
