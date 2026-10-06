// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'display.dart';
import 'foundation/geometry.dart';
import 'support.dart';
import 'unsupported.dart';

class DisplayManager {
  const DisplayManager._();

  static const DisplayManager instance = DisplayManager._();

  List<Display> getAll() => unsupported();

  Display? getPrimary() => unsupported();

  Point getCursorPosition() => unsupported();

  ListenerId addListener(FutureOr<void> Function(DisplayEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
