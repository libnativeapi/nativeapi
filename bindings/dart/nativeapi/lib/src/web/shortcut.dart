// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'unsupported.dart';

typedef ShortcutId = int;

enum ShortcutScope {
  global(0),
  application(1);

  const ShortcutScope(this.value);
  final int value;

  static ShortcutScope fromValue(int value) => switch (value) {
    0 => ShortcutScope.global,
    1 => ShortcutScope.application,
    _ => ShortcutScope.global,
  };
}

class ShortcutOptions {
  const ShortcutOptions({
    required this.accelerator,
    this.callback,
    required this.description,
    required this.scope,
    required this.enabled,
  });

  final String? accelerator;
  final void Function()? callback;
  final String? description;
  final ShortcutScope scope;
  final bool enabled;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is ShortcutOptions &&
          other.accelerator == accelerator &&
          other.description == description &&
          other.scope == scope &&
          other.enabled == enabled);

  @override
  int get hashCode => Object.hash(accelerator, description, scope, enabled);

  @override
  String toString() =>
      'ShortcutOptions(accelerator: $accelerator, description: $description, scope: $scope, enabled: $enabled)';
}

/// One `ShortcutEvent`, in its concrete form.
sealed class ShortcutEvent {
  const ShortcutEvent();

  ShortcutId get shortcutId;
  String? get accelerator;
}

final class ShortcutActivatedEvent extends ShortcutEvent {
  const ShortcutActivatedEvent({
    required this.shortcutId,
    required this.accelerator,
  });

  @override
  final ShortcutId shortcutId;
  @override
  final String? accelerator;
}

final class ShortcutRegisteredEvent extends ShortcutEvent {
  const ShortcutRegisteredEvent({
    required this.shortcutId,
    required this.accelerator,
  });

  @override
  final ShortcutId shortcutId;
  @override
  final String? accelerator;
}

final class ShortcutUnregisteredEvent extends ShortcutEvent {
  const ShortcutUnregisteredEvent({
    required this.shortcutId,
    required this.accelerator,
  });

  @override
  final ShortcutId shortcutId;
  @override
  final String? accelerator;
}

final class ShortcutRegistrationFailedEvent extends ShortcutEvent {
  const ShortcutRegistrationFailedEvent({
    required this.shortcutId,
    required this.accelerator,
    required this.errorMessage,
  });

  @override
  final ShortcutId shortcutId;
  @override
  final String? accelerator;
  final String? errorMessage;
}

class Shortcut {
  Shortcut.fromHandle(this.nativeHandle);
  Shortcut.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static Shortcut? createWithIdAndOptions(
    ShortcutId id,
    ShortcutOptions options,
  ) => unsupported();

  static Shortcut? createWithIdAndAcceleratorAndCallback(
    ShortcutId id,
    String accelerator,
    void Function() callback,
  ) => unsupported();

  ShortcutId get id => unsupported();

  String? get accelerator => unsupported();

  String? get description => unsupported();

  set description(String value) => unsupported();

  ShortcutScope get scope => unsupported();

  set isEnabled(bool value) => unsupported();

  bool get isEnabled => unsupported();

  void invoke() => unsupported();

  void setCallback(void Function() callback) => unsupported();
}
