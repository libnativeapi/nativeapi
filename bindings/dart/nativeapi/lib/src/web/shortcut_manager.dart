// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'shortcut.dart';
import 'support.dart';
import 'unsupported.dart';

class ShortcutManager {
  const ShortcutManager._();

  static const ShortcutManager instance = ShortcutManager._();

  bool isSupported() => unsupported();

  Shortcut? registerWithAcceleratorAndCallback(
    String accelerator,
    void Function() callback,
  ) => unsupported();

  Shortcut? registerWithOptions(ShortcutOptions options) => unsupported();

  bool unregisterWithId(ShortcutId id) => unsupported();

  bool unregisterWithAccelerator(String accelerator) => unsupported();

  int unregisterAll() => unsupported();

  Shortcut? getWithId(ShortcutId id) => unsupported();

  Shortcut? getWithAccelerator(String accelerator) => unsupported();

  List<Shortcut> getAll() => unsupported();

  List<Shortcut> getByScope(ShortcutScope scope) => unsupported();

  bool isAvailable(String accelerator) => unsupported();

  bool isValidAccelerator(String accelerator) => unsupported();

  void setEnabled(bool enabled) => unsupported();

  bool isEnabled() => unsupported();

  void emitShortcutActivated(ShortcutId id, String accelerator) =>
      unsupported();

  ListenerId addListener(FutureOr<void> Function(ShortcutEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
