// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'foundation/keyboard.dart';

import 'support.dart';

import 'callbacks.dart';

class KeyboardMonitor {
  /// Adopts a handle returned by the C API and releases it when this
  /// object becomes unreachable.
  KeyboardMonitor.fromHandle(this.nativeHandle) {
    _finalizer.attach(this, nativeHandle, detach: this);
  }

  /// Wraps a handle owned elsewhere; releasing it stays the owner's job.
  KeyboardMonitor.borrowed(this.nativeHandle);

  /// The underlying handle-table entry.
  final int nativeHandle;

  static final Finalizer<int> _finalizer = Finalizer<int>(
    (handle) => c.native_keyboard_monitor_free(handle),
  );

  /// Releases the handle now instead of at collection.
  void dispose() {
    _finalizer.detach(this);
    c.native_keyboard_monitor_free(nativeHandle);
  }

  /// Creates a new `KeyboardMonitor`; returns null if the native side failed.
  static KeyboardMonitor? create() {
    final handle = c.native_keyboard_monitor_create();
    if (handle == 0) return null;
    return KeyboardMonitor.fromHandle(handle);
  }

  void start() {
    c.native_keyboard_monitor_start(nativeHandle);
  }

  void stop() {
    c.native_keyboard_monitor_stop(nativeHandle);
  }

  bool get isMonitoring {
    return c.native_keyboard_monitor_is_monitoring(nativeHandle);
  }

  /// Registers [callback] for every `KeyboardEvent` this `KeyboardMonitor` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(FutureOr<void> Function(KeyboardEvent) callback) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<c.native_keyboard_event_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<c.native_keyboard_event_t> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : KeyboardEvent.fromNative(event.ref),
              callback,
            ),
          );
        });
    return c.native_keyboard_monitor_add_listener_async(
      nativeHandle,
      callable.nativeFunction,
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_keyboard_monitor_remove_listener(nativeHandle, listenerId);
}
