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

class WindowManager {
  const WindowManager._();

  /// The shared instance backed by the native singleton.
  static const WindowManager instance = WindowManager._();

  Window? get(WindowId id) {
    final handle = c.native_window_manager_get(id);
    if (handle == 0) return null;
    return Window.fromHandle(handle);
  }

  List<Window> getAll() {
    final list = c.native_window_manager_get_all();
    final items = <Window>[];
    for (var i = 0; i < list.count; i++) {
      items.add(Window.fromHandle(list.windows[i]));
    }
    final listPointer = pkg_ffi.calloc<c.native_window_list_t>();
    listPointer.ref = list;
    // The handles now belong to `items`; free just the array.
    c.native_window_list_release(listPointer);
    pkg_ffi.calloc.free(listPointer);
    return items;
  }

  Window? getCurrent() {
    final handle = c.native_window_manager_get_current();
    if (handle == 0) return null;
    return Window.fromHandle(handle);
  }

  Window? getWindowAtPoint(Point point, WindowId excludedWindowId) {
    final pointPointer = point.allocNative();
    final handle = c.native_window_manager_get_window_at_point(
      pointPointer.ref,
      excludedWindowId,
    );
    Point.freeNative(pointPointer);
    if (handle == 0) return null;
    return Window.fromHandle(handle);
  }

  void setWillShowHook(void Function(int)? hook) {
    final hookCallable = hook == null
        ? null
        : ffi.NativeCallable<
            ffi.Void Function(ffi.UnsignedInt, ffi.Pointer<ffi.Void>)
          >.isolateLocal((int arg0, ffi.Pointer<ffi.Void> _) {
            hook(arg0);
          });
    c.native_window_manager_set_will_show_hook(
      hookCallable?.nativeFunction ?? ffi.nullptr,
      NativeCallbacks.userData(hookCallable),
      NativeCallbacks.release,
    );
  }

  void setWillHideHook(void Function(int)? hook) {
    final hookCallable = hook == null
        ? null
        : ffi.NativeCallable<
            ffi.Void Function(ffi.UnsignedInt, ffi.Pointer<ffi.Void>)
          >.isolateLocal((int arg0, ffi.Pointer<ffi.Void> _) {
            hook(arg0);
          });
    c.native_window_manager_set_will_hide_hook(
      hookCallable?.nativeFunction ?? ffi.nullptr,
      NativeCallbacks.userData(hookCallable),
      NativeCallbacks.release,
    );
  }

  bool hasWillShowHook() {
    return c.native_window_manager_has_will_show_hook();
  }

  bool hasWillHideHook() {
    return c.native_window_manager_has_will_hide_hook();
  }

  void handleWillShow(WindowId id) {
    c.native_window_manager_handle_will_show(id);
  }

  void handleWillHide(WindowId id) {
    c.native_window_manager_handle_will_hide(id);
  }

  bool callOriginalShow(WindowId id) {
    return c.native_window_manager_call_original_show(id);
  }

  bool callOriginalHide(WindowId id) {
    return c.native_window_manager_call_original_hide(id);
  }

  /// Registers [callback] for every `WindowEvent` this `WindowManager` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(FutureOr<void> Function(WindowEvent) callback) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<c.native_window_event_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<c.native_window_event_t> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : WindowEvent.fromNative(event.ref),
              callback,
            ),
          );
        });
    return c.native_window_manager_add_listener_async(
      callable.nativeFunction,
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_window_manager_remove_listener(listenerId);
}
