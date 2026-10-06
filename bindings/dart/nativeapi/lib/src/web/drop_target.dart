// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'drag_source.dart';
import 'foundation/geometry.dart';
import 'window.dart';
import 'support.dart';
import 'unsupported.dart';

/// One `DropTargetEvent`, in its concrete form.
sealed class DropTargetEvent {
  const DropTargetEvent();

  WindowId get windowId;
  Point get position;
}

final class DropTargetEnteredEvent extends DropTargetEvent {
  const DropTargetEnteredEvent({
    required this.windowId,
    required this.position,
  });

  @override
  final WindowId windowId;
  @override
  final Point position;
}

final class DropTargetMovedEvent extends DropTargetEvent {
  const DropTargetMovedEvent({required this.windowId, required this.position});

  @override
  final WindowId windowId;
  @override
  final Point position;
}

final class DropTargetExitedEvent extends DropTargetEvent {
  const DropTargetExitedEvent({required this.windowId, required this.position});

  @override
  final WindowId windowId;
  @override
  final Point position;
}

final class DropTargetDroppedEvent extends DropTargetEvent {
  const DropTargetDroppedEvent({
    required this.windowId,
    required this.position,
    required this.filePaths,
    required this.text,
  });

  @override
  final WindowId windowId;
  @override
  final Point position;
  final List<String> filePaths;
  final String? text;
}

class DropTarget {
  DropTarget.fromHandle(this.nativeHandle);
  DropTarget.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static DropTarget? create(Window? window) => unsupported();

  static bool isSupported() => unsupported();

  WindowId get windowId => unsupported();

  set dropOperation(DragOperation value) => unsupported();

  DragOperation get dropOperation => unsupported();

  bool get isActive => unsupported();

  ListenerId addListener(FutureOr<void> Function(DropTargetEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
