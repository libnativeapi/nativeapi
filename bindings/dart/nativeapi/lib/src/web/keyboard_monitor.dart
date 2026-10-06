// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/keyboard.dart';
import 'support.dart';
import 'unsupported.dart';

class KeyboardMonitor {
  KeyboardMonitor.fromHandle(this.nativeHandle);
  KeyboardMonitor.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static KeyboardMonitor? create() => unsupported();

  void start() => unsupported();

  void stop() => unsupported();

  bool get isMonitoring => unsupported();

  ListenerId addListener(FutureOr<void> Function(KeyboardEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
