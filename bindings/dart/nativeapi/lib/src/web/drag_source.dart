// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/geometry.dart';
import 'image.dart';
import 'window.dart';
import 'support.dart';
import 'unsupported.dart';

enum DragOperation {
  none(0),
  copy(1),
  move(2),
  link(3);

  const DragOperation(this.value);
  final int value;

  static DragOperation fromValue(int value) => switch (value) {
    0 => DragOperation.none,
    1 => DragOperation.copy,
    2 => DragOperation.move,
    3 => DragOperation.link,
    _ => DragOperation.none,
  };
}

/// One `DragSourceEvent`, in its concrete form.
sealed class DragSourceEvent {
  const DragSourceEvent();

  WindowId get windowId;
  Point get position;
}

final class DragSourceEndedEvent extends DragSourceEvent {
  const DragSourceEndedEvent({
    required this.windowId,
    required this.position,
    required this.operation,
  });

  @override
  final WindowId windowId;
  @override
  final Point position;
  final DragOperation operation;
}

class DragSource {
  DragSource.fromHandle(this.nativeHandle);
  DragSource.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static DragSource? create() => unsupported();

  static bool isSupported() => unsupported();

  set filePaths(List<String> value) => unsupported();

  List<String> get filePaths => unsupported();

  set text(String? value) => unsupported();

  String? get text => unsupported();

  set image(Image? value) => unsupported();

  Image? get image => unsupported();

  set dragOperation(DragOperation value) => unsupported();

  DragOperation get dragOperation => unsupported();

  bool startDragging(Window? window) => unsupported();

  bool get isDragging => unsupported();

  ListenerId addListener(FutureOr<void> Function(DragSourceEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
