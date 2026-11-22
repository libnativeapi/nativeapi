// Self-driving Windows 11 regression for #50. No mouse or keyboard input.
import 'dart:async';
import 'dart:convert';
import 'dart:ffi';
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:flutter/widgets.dart';
import 'package:nativeapi/nativeapi.dart' as native;

final _attribute = DynamicLibrary.open('dwmapi.dll')
    .lookupFunction<
      Int32 Function(Pointer<Void>, Uint32, Pointer<Void>, Uint32),
      int Function(Pointer<Void>, int, Pointer<Void>, int)
    >('DwmGetWindowAttribute');

int attribute(native.Window window, int attribute) {
  final value = calloc<Uint32>();
  try {
    final result = _attribute(
      window.nativeObject,
      attribute,
      value.cast(),
      sizeOf<Uint32>(),
    );
    if (result != 0)
      throw StateError('DwmGetWindowAttribute($attribute): $result');
    return value.value;
  } finally {
    calloc.free(value);
  }
}

final class _Rect extends Struct {
  @Int32()
  external int left;
  @Int32()
  external int top;
  @Int32()
  external int right;
  @Int32()
  external int bottom;
}

final _user32 = DynamicLibrary.open('user32.dll');
final _windowRect = _user32
    .lookupFunction<
      Int32 Function(Pointer<Void>, Pointer<_Rect>),
      int Function(Pointer<Void>, Pointer<_Rect>)
    >('GetWindowRect');
final _clientRect = _user32
    .lookupFunction<
      Int32 Function(Pointer<Void>, Pointer<_Rect>),
      int Function(Pointer<Void>, Pointer<_Rect>)
    >('GetClientRect');
final _getDC = _user32
    .lookupFunction<
      Pointer<Void> Function(Pointer<Void>),
      Pointer<Void> Function(Pointer<Void>)
    >('GetDC');
final _releaseDC = _user32
    .lookupFunction<
      Int32 Function(Pointer<Void>, Pointer<Void>),
      int Function(Pointer<Void>, Pointer<Void>)
    >('ReleaseDC');
final _getPixel = DynamicLibrary.open('gdi32.dll')
    .lookupFunction<
      Uint32 Function(Pointer<Void>, Int32, Int32),
      int Function(Pointer<Void>, int, int)
    >('GetPixel');

List<int> framePixels(native.Window window) {
  final rect = calloc<_Rect>();
  final dc = _getDC(nullptr);
  try {
    if (_windowRect(window.nativeObject, rect) == 0 || dc == nullptr)
      throw StateError('Cannot measure the test window');
    final r = rect.ref;
    final x = (r.left + r.right) ~/ 2;
    final y = (r.top + r.bottom) ~/ 2;
    return [
      _getPixel(dc, x, r.top),
      _getPixel(dc, r.right - 1, y),
      _getPixel(dc, x, r.bottom - 1),
      _getPixel(dc, r.left, y),
    ];
  } finally {
    _releaseDC(nullptr, dc);
    calloc.free(rect);
  }
}

bool clientFillsFrame(native.Window window) {
  final outer = calloc<_Rect>();
  final client = calloc<_Rect>();
  try {
    if (_windowRect(window.nativeObject, outer) == 0 ||
        _clientRect(window.nativeObject, client) == 0)
      return false;
    return outer.ref.right - outer.ref.left == client.ref.right &&
        outer.ref.bottom - outer.ref.top == client.ref.bottom;
  } finally {
    calloc.free(outer);
    calloc.free(client);
  }
}

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final window = native.WindowManager.instance.getCurrent();
  if (window == null) throw StateError('No Flutter host window');
  window
    ..title = 'NativeAPI frameless focus regression'
    ..backgroundColor = const native.Color(a: 255, r: 0, g: 0, b: 0)
    ..titleBarStyle = native.TitleBarStyle.hidden
    ..minimumSize = const native.Size(width: 800, height: 600);
  if (!window.setFrameless(true)) throw StateError('SetFrameless failed');
  // Let the native close complete after the FFI call returns to Dart.
  window.addListener((_) {});
  window.show();
  runApp(
    const Directionality(
      textDirection: TextDirection.ltr,
      child: ColoredBox(
        color: Color(0xFF000000),
        child: Center(child: Text('Frameless focus regression')),
      ),
    ),
  );
  await WidgetsBinding.instance.endOfFrame;
  final anchor = native.Window.create()!
    ..title = 'NativeAPI focus anchor'
    ..bounds = native.Rectangle(
      x: window.bounds.x + 100,
      y: window.bounds.y + 100,
      width: 250,
      height: 150,
    )
    ..show();
  var passed = 0;
  final failures = <String>[];
  void check(bool ok, String label) {
    stdout.writeln('${ok ? "PASS" : "FAIL"} $label');
    if (ok) {
      passed++;
    } else {
      failures.add(label);
    }
  }

  void checkFrame(String phase) {
    check(window.isFrameless, '$phase remains frameless');
    // DWMWA_NCRENDERING_ENABLED is readable. DWMWA_BORDER_COLOR is set-only,
    // so inspect the actual desktop pixels at the midpoints of all four edges.
    check(
      attribute(window, 1) == 0,
      '$phase DWM non-client rendering is disabled',
    );
    check(clientFillsFrame(window), '$phase client fills the whole frame');
    final pixels = framePixels(window);
    check(
      pixels.every((color) => color == 0),
      '$phase edges stay black: $pixels',
    );
  }

  try {
    for (var cycle = 1; cycle <= 3; cycle++) {
      anchor.focus();
      await Future<void>.delayed(const Duration(milliseconds: 400));
      check(
        anchor.isFocused && !window.isFocused,
        'cycle $cycle focuses the owned anchor',
      );
      checkFrame('inactive $cycle');
      window.focus();
      await Future<void>.delayed(const Duration(milliseconds: 400));
      check(
        window.isFocused && !anchor.isFocused,
        'cycle $cycle focuses the Flutter host',
      );
      checkFrame('active $cycle');
    }
    window.setFrameless(false);
    await Future<void>.delayed(const Duration(milliseconds: 300));
    check(!window.isFrameless, 'normal frame is restored');
    check(attribute(window, 1) != 0, 'normal DWM frame is restored');
    check(!clientFillsFrame(window), 'normal non-client frame is restored');
  } catch (error, stack) {
    failures.add('$error');
    stderr.writeln('$error\n$stack');
  } finally {
    anchor.close();
    stdout.writeln(
      'FRAMELESS_RESULT ${jsonEncode({'passed': passed, 'failures': failures})}',
    );
    window.close();
  }
}
