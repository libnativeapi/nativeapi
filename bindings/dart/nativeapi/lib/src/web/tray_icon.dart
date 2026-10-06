// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/geometry.dart';
import 'image.dart';
import 'menu.dart';
import 'view.dart';
import 'support.dart';
import 'unsupported.dart';

typedef TrayIconId = int;

enum ContextMenuTrigger {
  none(0),
  clicked(1),
  rightClicked(2),
  doubleClicked(3);

  const ContextMenuTrigger(this.value);
  final int value;

  static ContextMenuTrigger fromValue(int value) => switch (value) {
    0 => ContextMenuTrigger.none,
    1 => ContextMenuTrigger.clicked,
    2 => ContextMenuTrigger.rightClicked,
    3 => ContextMenuTrigger.doubleClicked,
    _ => ContextMenuTrigger.none,
  };
}

enum TrayIconPosition {
  left(0),
  right(1);

  const TrayIconPosition(this.value);
  final int value;

  static TrayIconPosition fromValue(int value) => switch (value) {
    0 => TrayIconPosition.left,
    1 => TrayIconPosition.right,
    _ => TrayIconPosition.left,
  };
}

/// One `TrayIconEvent`, in its concrete form.
sealed class TrayIconEvent {
  const TrayIconEvent();
}

final class TrayIconClickedEvent extends TrayIconEvent {
  const TrayIconClickedEvent({required this.trayIconId});

  final TrayIconId trayIconId;
}

final class TrayIconRightClickedEvent extends TrayIconEvent {
  const TrayIconRightClickedEvent({required this.trayIconId});

  final TrayIconId trayIconId;
}

final class TrayIconDoubleClickedEvent extends TrayIconEvent {
  const TrayIconDoubleClickedEvent({required this.trayIconId});

  final TrayIconId trayIconId;
}

class TrayIcon {
  TrayIcon.fromHandle(this.nativeHandle);
  TrayIcon.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static TrayIcon? create() => unsupported();

  static TrayIcon? createWithIdentifier(String identifier) => unsupported();

  TrayIconId getId() => unsupported();

  String? get identifier => unsupported();

  set icon(Image? value) => unsupported();

  Image? get icon => unsupported();

  set isIconTemplate(bool value) => unsupported();

  bool get isIconTemplate => unsupported();

  set iconSize(Size value) => unsupported();

  Size get iconSize => unsupported();

  set iconPosition(TrayIconPosition value) => unsupported();

  TrayIconPosition get iconPosition => unsupported();

  void setTitle(String? title) => unsupported();

  String? getTitle() => unsupported();

  void setTooltip(String? tooltip) => unsupported();

  String? getTooltip() => unsupported();

  set contentView(View? value) => unsupported();

  View? get contentView => unsupported();

  void setContextMenu(Menu? menu) => unsupported();

  Menu? getContextMenu() => unsupported();

  void setContextMenuTrigger(ContextMenuTrigger trigger) => unsupported();

  ContextMenuTrigger getContextMenuTrigger() => unsupported();

  Rectangle getBounds() => unsupported();

  bool setVisible(bool visible) => unsupported();

  bool isVisible() => unsupported();

  bool openContextMenu() => unsupported();

  bool closeContextMenu() => unsupported();

  ListenerId addListener(FutureOr<void> Function(TrayIconEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
