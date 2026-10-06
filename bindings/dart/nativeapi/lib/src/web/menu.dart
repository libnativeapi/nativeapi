// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/keyboard.dart';
import 'image.dart';
import 'placement.dart';
import 'positioning_strategy.dart';
import 'support.dart';
import 'unsupported.dart';

typedef MenuId = int;

typedef MenuItemId = int;

enum MenuBackend {
  native(0),
  winUi3(1);

  const MenuBackend(this.value);
  final int value;

  static MenuBackend fromValue(int value) => switch (value) {
    0 => MenuBackend.native,
    1 => MenuBackend.winUi3,
    _ => MenuBackend.native,
  };
}

enum MenuItemType {
  normal(0),
  checkbox(1),
  radio(2),
  separator(3),
  submenu(4);

  const MenuItemType(this.value);
  final int value;

  static MenuItemType fromValue(int value) => switch (value) {
    0 => MenuItemType.normal,
    1 => MenuItemType.checkbox,
    2 => MenuItemType.radio,
    3 => MenuItemType.separator,
    4 => MenuItemType.submenu,
    _ => MenuItemType.normal,
  };
}

enum MenuItemState {
  unchecked(0),
  checked(1),
  mixed(2);

  const MenuItemState(this.value);
  final int value;

  static MenuItemState fromValue(int value) => switch (value) {
    0 => MenuItemState.unchecked,
    1 => MenuItemState.checked,
    2 => MenuItemState.mixed,
    _ => MenuItemState.unchecked,
  };
}

/// One `MenuEvent`, in its concrete form.
sealed class MenuEvent {
  const MenuEvent();
}

final class MenuOpenedEvent extends MenuEvent {
  const MenuOpenedEvent({required this.menuId});

  final MenuId menuId;
}

final class MenuClosedEvent extends MenuEvent {
  const MenuClosedEvent({required this.menuId});

  final MenuId menuId;
}

final class MenuItemClickedEvent extends MenuEvent {
  const MenuItemClickedEvent({required this.itemId});

  final MenuItemId itemId;
}

final class MenuItemSubmenuOpenedEvent extends MenuEvent {
  const MenuItemSubmenuOpenedEvent({required this.itemId});

  final MenuItemId itemId;
}

final class MenuItemSubmenuClosedEvent extends MenuEvent {
  const MenuItemSubmenuClosedEvent({required this.itemId});

  final MenuItemId itemId;
}

class MenuItem {
  MenuItem.fromHandle(this.nativeHandle);
  MenuItem.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static MenuItem? createWithLabelAndType(String label, MenuItemType type) =>
      unsupported();

  MenuItemId get id => unsupported();

  MenuItemType get type => unsupported();

  set label(String? value) => unsupported();

  String? get label => unsupported();

  set icon(Image? value) => unsupported();

  Image? get icon => unsupported();

  set tooltip(String? value) => unsupported();

  String? get tooltip => unsupported();

  set accelerator(KeyboardAccelerator? value) => unsupported();

  KeyboardAccelerator get accelerator => unsupported();

  set isEnabled(bool value) => unsupported();

  bool get isEnabled => unsupported();

  set state(MenuItemState value) => unsupported();

  MenuItemState get state => unsupported();

  set radioGroup(int value) => unsupported();

  int get radioGroup => unsupported();

  set submenu(Menu? value) => unsupported();

  Menu? get submenu => unsupported();

  ListenerId addListener(FutureOr<void> Function(MenuEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}

class Menu {
  Menu.fromHandle(this.nativeHandle);
  Menu.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static Menu? create() => unsupported();

  MenuId get id => unsupported();

  bool setBackend(MenuBackend backend) => unsupported();

  MenuBackend get backend => unsupported();

  static bool isBackendSupported(MenuBackend backend) => unsupported();

  void addItem(MenuItem? item) => unsupported();

  void insertItem(int index, MenuItem? item) => unsupported();

  bool removeItem(MenuItem? item) => unsupported();

  bool removeItemById(MenuItemId itemId) => unsupported();

  bool removeItemAt(int index) => unsupported();

  void clear() => unsupported();

  void addSeparator() => unsupported();

  void insertSeparator(int index) => unsupported();

  int get itemCount => unsupported();

  MenuItem? getItemAt(int index) => unsupported();

  MenuItem? getItemById(MenuItemId itemId) => unsupported();

  List<MenuItem> get allItems => unsupported();

  bool open(PositioningStrategy strategy, Placement placement) => unsupported();

  bool close() => unsupported();

  ListenerId addListener(FutureOr<void> Function(MenuEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
