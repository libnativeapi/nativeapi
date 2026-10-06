// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'foundation/geometry.dart';
import 'window.dart';
import 'unsupported.dart';

enum PositioningStrategyType {
  absolute(0),
  cursorPosition(1),
  relative(2);

  const PositioningStrategyType(this.value);
  final int value;

  static PositioningStrategyType fromValue(int value) => switch (value) {
    0 => PositioningStrategyType.absolute,
    1 => PositioningStrategyType.cursorPosition,
    2 => PositioningStrategyType.relative,
    _ => PositioningStrategyType.absolute,
  };
}

class PositioningStrategy {
  PositioningStrategy.fromHandle(this.nativeHandle);
  PositioningStrategy.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static PositioningStrategy? absolute(Point point) => unsupported();

  static PositioningStrategy? cursorPosition() => unsupported();

  static PositioningStrategy? relativeWithRectAndOffset(
    Rectangle rect,
    Point offset,
  ) => unsupported();

  static PositioningStrategy? relativeWithWindowAndOffset(
    Window window,
    Point offset,
  ) => unsupported();

  PositioningStrategyType get type => unsupported();

  Point get absolutePosition => unsupported();

  Rectangle get relativeRectangle => unsupported();

  Point get relativeOffset => unsupported();
}
