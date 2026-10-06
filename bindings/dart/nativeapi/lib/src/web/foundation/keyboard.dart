// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

enum ModifierKey {
  none(0),
  shift(1),
  ctrl(2),
  alt(4),
  meta(8),
  fn(16),
  capsLock(32),
  numLock(64),
  scrollLock(128);

  const ModifierKey(this.value);
  final int value;

  static ModifierKey fromValue(int value) => switch (value) {
    0 => ModifierKey.none,
    1 => ModifierKey.shift,
    2 => ModifierKey.ctrl,
    4 => ModifierKey.alt,
    8 => ModifierKey.meta,
    16 => ModifierKey.fn,
    32 => ModifierKey.capsLock,
    64 => ModifierKey.numLock,
    128 => ModifierKey.scrollLock,
    _ => ModifierKey.none,
  };
}

class KeyboardAccelerator {
  const KeyboardAccelerator({required this.modifiers, required this.key});

  final ModifierKey modifiers;
  final String? key;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is KeyboardAccelerator &&
          other.modifiers == modifiers &&
          other.key == key);

  @override
  int get hashCode => Object.hash(modifiers, key);

  @override
  String toString() => 'KeyboardAccelerator(modifiers: $modifiers, key: $key)';
}

/// One `KeyboardEvent`, in its concrete form.
sealed class KeyboardEvent {
  const KeyboardEvent();

  int get keycode;
}

final class KeyPressedEvent extends KeyboardEvent {
  const KeyPressedEvent({required this.keycode});

  @override
  final int keycode;
}

final class KeyReleasedEvent extends KeyboardEvent {
  const KeyReleasedEvent({required this.keycode});

  @override
  final int keycode;
}

final class ModifierKeysChangedEvent extends KeyboardEvent {
  const ModifierKeysChangedEvent({
    required this.keycode,
    required this.modifierKeys,
  });

  @override
  final int keycode;
  final int modifierKeys;
}
