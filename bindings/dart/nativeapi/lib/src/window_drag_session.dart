// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'foundation/geometry.dart';
import 'window.dart';

import 'support.dart';

import 'callbacks.dart';

/// One `WindowDragEvent`, in its concrete form.
sealed class WindowDragEvent {
  const WindowDragEvent();

  WindowId get windowId;
  Point get cursorPosition;

  /// Reads the event out of its C form. Returns null for a variant this
  /// binding does not know about.
  static WindowDragEvent? fromNative(c.native_window_drag_event_t raw) {
    if (raw.typeAsInt ==
        c
            .native_window_drag_event_type_t
            .NATIVE_WINDOW_DRAG_EVENT_TYPE_MOVED
            .value) {
      return WindowDragMovedEvent(
        windowId: raw.window_id,
        cursorPosition: Point.fromNative(raw.cursor_position),
      );
    }
    if (raw.typeAsInt ==
        c
            .native_window_drag_event_type_t
            .NATIVE_WINDOW_DRAG_EVENT_TYPE_ENDED
            .value) {
      return WindowDragEndedEvent(
        windowId: raw.window_id,
        cursorPosition: Point.fromNative(raw.cursor_position),
      );
    }
    if (raw.typeAsInt ==
        c
            .native_window_drag_event_type_t
            .NATIVE_WINDOW_DRAG_EVENT_TYPE_CANCELLED
            .value) {
      return WindowDragCancelledEvent(
        windowId: raw.window_id,
        cursorPosition: Point.fromNative(raw.cursor_position),
      );
    }
    return null;
  }
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

class WindowDragSession implements ffi.Finalizable {
  /// Adopts a handle returned by the C API and releases it when this
  /// object becomes unreachable.
  WindowDragSession.fromHandle(this.nativeHandle) {
    _finalizer.attach(
      this,
      ffi.Pointer<ffi.Void>.fromAddress(nativeHandle),
      detach: this,
    );
  }

  /// Wraps a handle owned elsewhere; releasing it stays the owner's job.
  WindowDragSession.borrowed(this.nativeHandle);

  /// The underlying handle-table entry.
  final int nativeHandle;

  static final _finalizer = ffi.NativeFinalizer(
    ffi.Native.addressOf<
      ffi.NativeFunction<ffi.Void Function(ffi.Pointer<ffi.Void>)>
    >(c.native_handle_finalize),
  );

  /// Releases the handle now instead of at collection.
  void dispose() {
    _finalizer.detach(this);
    c.native_window_drag_session_free(nativeHandle);
  }

  /// Creates a new `WindowDragSession`; returns null if the native side failed.
  static WindowDragSession? create() {
    final handle = c.native_window_drag_session_create();
    if (handle == 0) return null;
    return WindowDragSession.fromHandle(handle);
  }

  bool start(Window? window, Point anchor) {
    final anchorPointer = anchor.allocNative();
    final result = c.native_window_drag_session_start(
      nativeHandle,
      window?.nativeHandle ?? 0,
      anchorPointer.ref,
    );
    Point.freeNative(anchorPointer);
    return result;
  }

  void cancel() {
    c.native_window_drag_session_cancel(nativeHandle);
  }

  bool get isActive {
    return c.native_window_drag_session_is_active(nativeHandle);
  }

  WindowId get windowId {
    return c.native_window_drag_session_get_window_id(nativeHandle);
  }

  Point get anchor {
    final raw = c.native_window_drag_session_get_anchor(nativeHandle);
    return Point.fromNative(raw);
  }

  /// Registers [callback] for every `WindowDragEvent` this `WindowDragSession` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(FutureOr<void> Function(WindowDragEvent) callback) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<c.native_window_drag_event_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<c.native_window_drag_event_t> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : WindowDragEvent.fromNative(event.ref),
              callback,
            ),
          );
        });
    return c.native_window_drag_session_add_listener_async(
      nativeHandle,
      callable.nativeFunction,
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_window_drag_session_remove_listener(nativeHandle, listenerId);
}
