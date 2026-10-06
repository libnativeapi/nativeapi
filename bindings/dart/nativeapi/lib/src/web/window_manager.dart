// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/geometry.dart';
import 'window.dart';
import 'support.dart';
import 'unsupported.dart';

class WindowManager {
  const WindowManager._();

  static const WindowManager instance = WindowManager._();

  Window? get(WindowId id) => unsupported();

  List<Window> getAll() => unsupported();

  Window? getCurrent() => unsupported();

  Window? getWindowAtPoint(Point point, WindowId excludedWindowId) =>
      unsupported();

  void setWillShowHook(void Function(int)? hook) => unsupported();

  void setWillHideHook(void Function(int)? hook) => unsupported();

  bool hasWillShowHook() => unsupported();

  bool hasWillHideHook() => unsupported();

  void handleWillShow(WindowId id) => unsupported();

  void handleWillHide(WindowId id) => unsupported();

  bool callOriginalShow(WindowId id) => unsupported();

  bool callOriginalHide(WindowId id) => unsupported();

  ListenerId addListener(FutureOr<void> Function(WindowEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
