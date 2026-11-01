// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'display.dart';
import 'window.dart';
import 'unsupported.dart';

enum CaptureSourceType {
  none(0),
  display(1),
  window(2),
  unknown(3);

  const CaptureSourceType(this.value);
  final int value;

  static CaptureSourceType fromValue(int value) => switch (value) {
    0 => CaptureSourceType.none,
    1 => CaptureSourceType.display,
    2 => CaptureSourceType.window,
    3 => CaptureSourceType.unknown,
    _ => CaptureSourceType.none,
  };
}

class CaptureSource {
  CaptureSource.fromHandle(this.nativeHandle);
  CaptureSource.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static CaptureSource? fromDisplay(Display? display) => unsupported();

  static CaptureSource? fromWindow(Window? window) => unsupported();

  CaptureSourceType get type => unsupported();

  String? get name => unsupported();
}
