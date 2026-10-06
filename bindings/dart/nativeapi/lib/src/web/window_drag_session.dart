// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/geometry.dart';
import 'window.dart';
import 'support.dart';
import 'unsupported.dart';

/// One `WindowDragEvent`, in its concrete form.
sealed class WindowDragEvent {
  const WindowDragEvent();

  WindowId get windowId;
  Point get cursorPosition;
}

final class WindowDragMovedEvent extends WindowDragEvent {
  const WindowDragMovedEvent({
    required this.windowId,
    required this.cursorPosition,
  });

  @override
  final WindowId windowId;
  @override
  final Point cursorPosition;
}

final class WindowDragEndedEvent extends WindowDragEvent {
  const WindowDragEndedEvent({
    required this.windowId,
    required this.cursorPosition,
  });

  @override
  final WindowId windowId;
  @override
  final Point cursorPosition;
}

final class WindowDragCancelledEvent extends WindowDragEvent {
  const WindowDragCancelledEvent({
    required this.windowId,
    required this.cursorPosition,
  });

  @override
  final WindowId windowId;
  @override
  final Point cursorPosition;
}

class WindowDragSession {
  WindowDragSession.fromHandle(this.nativeHandle);
  WindowDragSession.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static WindowDragSession? create() => unsupported();

  bool start(Window? window, Point anchor) => unsupported();

  void cancel() => unsupported();

  bool get isActive => unsupported();

  WindowId get windowId => unsupported();

  Point get anchor => unsupported();

  ListenerId addListener(FutureOr<void> Function(WindowDragEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
