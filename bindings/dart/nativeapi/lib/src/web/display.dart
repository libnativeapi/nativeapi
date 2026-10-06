// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'foundation/geometry.dart';
import 'unsupported.dart';

typedef DisplayId = int;

enum DisplayOrientation {
  portrait(0),
  landscape(90),
  portraitFlipped(180),
  landscapeFlipped(270);

  const DisplayOrientation(this.value);
  final int value;

  static DisplayOrientation fromValue(int value) => switch (value) {
    0 => DisplayOrientation.portrait,
    90 => DisplayOrientation.landscape,
    180 => DisplayOrientation.portraitFlipped,
    270 => DisplayOrientation.landscapeFlipped,
    _ => DisplayOrientation.portrait,
  };
}

/// One `DisplayEvent`, in its concrete form.
sealed class DisplayEvent {
  const DisplayEvent();

  Display get display;
}

final class DisplayAddedEvent extends DisplayEvent {
  const DisplayAddedEvent({required this.display});

  @override
  final Display display;
}

final class DisplayRemovedEvent extends DisplayEvent {
  const DisplayRemovedEvent({required this.display});

  @override
  final Display display;
}

final class DisplayChangedEvent extends DisplayEvent {
  const DisplayChangedEvent({required this.display});

  @override
  final Display display;
}

class Display {
  Display.fromHandle(this.nativeHandle);
  Display.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  DisplayId get id => unsupported();

  String? get name => unsupported();

  Point get position => unsupported();

  Size get size => unsupported();

  Rectangle get workArea => unsupported();

  double get scaleFactor => unsupported();

  bool get isPrimary => unsupported();

  DisplayOrientation get orientation => unsupported();

  int get refreshRate => unsupported();

  int get bitDepth => unsupported();
}
