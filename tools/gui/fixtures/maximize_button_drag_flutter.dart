// Regression fixture for nativeapi#23. Build with the title-bar example's runner:
// flutter build windows --debug -t <workspace>/tools/gui/fixtures/maximize_button_drag_flutter.dart
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show MaximizeButtonArea;

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  final window = na.WindowManager.instance.getCurrent()!;
  window
    ..title = 'Maximize drag regression'
    ..titleBarStyle = na.TitleBarStyle.hidden
    ..isWindowControlButtonsVisible = false
    ..contentSize = const na.Size(width: 760, height: 560);
  window.center();
  runApp(_App(window: window));
}

// Count the real Material splash lifetime without changing its behavior.
class _CountingSplashFactory extends InteractiveInkFeatureFactory {
  _CountingSplashFactory(this.changed);

  final void Function(int delta) changed;

  @override
  InteractiveInkFeature create({
    required MaterialInkController controller,
    required RenderBox referenceBox,
    required Offset position,
    required Color color,
    required TextDirection textDirection,
    bool containedInkWell = false,
    RectCallback? rectCallback,
    BorderRadius? borderRadius,
    ShapeBorder? customBorder,
    double? radius,
    VoidCallback? onRemoved,
  }) {
    changed(1);
    return InkSplash.splashFactory.create(
      controller: controller,
      referenceBox: referenceBox,
      position: position,
      color: color,
      textDirection: textDirection,
      containedInkWell: containedInkWell,
      rectCallback: rectCallback,
      borderRadius: borderRadius,
      customBorder: customBorder,
      radius: radius,
      onRemoved: () {
        changed(-1);
        onRemoved?.call();
      },
    );
  }
}

class _App extends StatefulWidget {
  const _App({required this.window});

  final na.Window window;

  @override
  State<_App> createState() => _AppState();
}

class _AppState extends State<_App> {
  final _pointers = <int>{};
  final _pressed = <String>{};
  final _taps = List<int>.filled(6, 0);
  int _downs = 0;
  int _ups = 0;
  int _cancels = 0;
  int _splashes = 0;
  int _toggles = 0;
  late final _splashFactory = _CountingSplashFactory((delta) {
    _splashes += delta;
  });
  late final Timer _refresh;

  @override
  void initState() {
    super.initState();
    // Ink features can be removed during painting. Refresh diagnostics outside
    // paint instead of calling setState from their disposal callback.
    _refresh = Timer.periodic(const Duration(milliseconds: 100), (_) {
      if (mounted) setState(() {});
    });
  }

  @override
  void dispose() {
    _refresh.cancel();
    super.dispose();
  }

  Widget _button(String label, String id, VoidCallback onTap) {
    return Material(
      color: Colors.blueGrey.shade50,
      child: InkWell(
        splashFactory: _splashFactory,
        onTap: onTap,
        onHighlightChanged: (pressed) {
          pressed ? _pressed.add(id) : _pressed.remove(id);
        },
        child: Center(child: Text(label)),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Scaffold(
        body: Listener(
          onPointerDown: (event) {
            _downs++;
            _pointers.add(event.pointer);
          },
          onPointerUp: (event) {
            _ups++;
            _pointers.remove(event.pointer);
          },
          onPointerCancel: (event) {
            _cancels++;
            _pointers.remove(event.pointer);
          },
          child: Column(
            children: [
              SizedBox(
                height: 48,
                child: Row(
                  children: [
                    const Expanded(child: Center(child: Text('Drag fixture'))),
                    SizedBox(
                      width: 120,
                      child: MaximizeButtonArea(
                        window: widget.window,
                        child: _button(
                          widget.window.isMaximized ? 'Restore' : 'Maximize',
                          'maximize',
                          () {
                            _toggles++;
                            widget.window.isMaximized
                                ? widget.window.unmaximize()
                                : widget.window.maximize();
                          },
                        ),
                      ),
                    ),
                    const SizedBox(width: 48),
                  ],
                ),
              ),
              Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  children: [
                    Text('Pointers: ${_pointers.length}'),
                    Text('Downs: $_downs  Ups: $_ups  Cancels: $_cancels'),
                    Text('Pressed: ${_pressed.length}'),
                    Text('Splashes: $_splashes'),
                    Text('Toggles: $_toggles'),
                  ],
                ),
              ),
              Expanded(
                child: Padding(
                  padding: const EdgeInsets.all(16),
                  child: Column(
                    children: [
                      for (var row = 0; row < 2; row++)
                        Expanded(
                          child: Row(
                            children: [
                              for (var column = 0; column < 3; column++)
                                Expanded(
                                  child: Padding(
                                    padding: const EdgeInsets.all(8),
                                    child: _button(
                                      'Card ${row * 3 + column + 1}: '
                                          '${_taps[row * 3 + column]}',
                                      'card-${row * 3 + column}',
                                      () => _taps[row * 3 + column]++,
                                    ),
                                  ),
                                ),
                            ],
                          ),
                        ),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
