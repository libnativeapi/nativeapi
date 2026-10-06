// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'display.dart';
import 'foundation/geometry.dart';

import 'support.dart';

import 'callbacks.dart';

class DisplayManager {
  const DisplayManager._();

  /// The shared instance backed by the native singleton.
  static const DisplayManager instance = DisplayManager._();

  List<Display> getAll() {
    final list = c.native_display_manager_get_all();
    final items = <Display>[];
    for (var i = 0; i < list.count; i++) {
      items.add(Display.fromHandle(list.displays[i]));
    }
    final listPointer = pkg_ffi.calloc<c.native_display_list_t>();
    listPointer.ref = list;
    // The handles now belong to `items`; free just the array.
    c.native_display_list_release(listPointer);
    pkg_ffi.calloc.free(listPointer);
    return items;
  }

  Display? getPrimary() {
    final handle = c.native_display_manager_get_primary();
    if (handle == 0) return null;
    return Display.fromHandle(handle);
  }

  Point getCursorPosition() {
    final raw = c.native_display_manager_get_cursor_position();
    return Point.fromNative(raw);
  }

  /// Registers [callback] for every `DisplayEvent` this `DisplayManager` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(FutureOr<void> Function(DisplayEvent) callback) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<c.native_display_event_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<c.native_display_event_t> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : DisplayEvent.fromNative(event.ref),
              callback,
            ),
          );
        });
    return c.native_display_manager_add_listener_async(
      callable.nativeFunction,
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_display_manager_remove_listener(listenerId);
}
