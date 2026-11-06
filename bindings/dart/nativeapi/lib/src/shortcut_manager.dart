// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'shortcut.dart';

import 'support.dart';

import 'callbacks.dart';

class ShortcutManager {
  const ShortcutManager._();

  /// The shared instance backed by the native singleton.
  static const ShortcutManager instance = ShortcutManager._();

  bool isSupported() {
    return c.native_shortcut_manager_is_supported();
  }

  Shortcut? registerWithAcceleratorAndCallback(
    String accelerator,
    void Function() callback,
  ) {
    final acceleratorNative = accelerator.toNativeUtf8().cast<ffi.Char>();
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(ffi.Pointer<ffi.Void>)
        >.isolateLocal((ffi.Pointer<ffi.Void> _) {
          callback();
        });
    final handle = c
        .native_shortcut_manager_register_with_accelerator_and_callback(
          acceleratorNative,
          callbackCallable.nativeFunction.cast(),
          NativeCallbacks.userData(callbackCallable),
          NativeCallbacks.release,
        );
    pkg_ffi.calloc.free(acceleratorNative);
    if (handle == 0) return null;
    return Shortcut.fromHandle(handle);
  }

  Shortcut? registerWithOptions(ShortcutOptions options) {
    final optionsPointer = options.allocNative();
    final handle = c.native_shortcut_manager_register_with_options(
      optionsPointer.ref,
    );
    ShortcutOptions.freeNative(optionsPointer);
    if (handle == 0) return null;
    return Shortcut.fromHandle(handle);
  }

  ShortcutId requestRegistration(
    ShortcutOptions options,
    String registrationKey,
  ) {
    final optionsPointer = options.allocNative();
    final registrationKeyNative = registrationKey
        .toNativeUtf8()
        .cast<ffi.Char>();
    final result = c.native_shortcut_manager_request_registration(
      optionsPointer.ref,
      registrationKeyNative,
    );
    ShortcutOptions.freeNative(optionsPointer);
    pkg_ffi.calloc.free(registrationKeyNative);
    return result;
  }

  bool cancelRegistration(ShortcutId requestId) {
    return c.native_shortcut_manager_cancel_registration(requestId);
  }

  int cancelPendingRegistrations(String registrationKeyPrefix) {
    final registrationKeyPrefixNative = registrationKeyPrefix
        .toNativeUtf8()
        .cast<ffi.Char>();
    final result = c.native_shortcut_manager_cancel_pending_registrations(
      registrationKeyPrefixNative,
    );
    pkg_ffi.calloc.free(registrationKeyPrefixNative);
    return result;
  }

  bool isRegistrationPending(ShortcutId requestId) {
    return c.native_shortcut_manager_is_registration_pending(requestId);
  }

  bool unregisterWithId(ShortcutId id) {
    return c.native_shortcut_manager_unregister_with_id(id);
  }

  bool unregisterWithAccelerator(String accelerator) {
    final acceleratorNative = accelerator.toNativeUtf8().cast<ffi.Char>();
    final result = c.native_shortcut_manager_unregister_with_accelerator(
      acceleratorNative,
    );
    pkg_ffi.calloc.free(acceleratorNative);
    return result;
  }

  int unregisterAll() {
    return c.native_shortcut_manager_unregister_all();
  }

  Shortcut? getWithId(ShortcutId id) {
    final handle = c.native_shortcut_manager_get_with_id(id);
    if (handle == 0) return null;
    return Shortcut.fromHandle(handle);
  }

  Shortcut? getWithAccelerator(String accelerator) {
    final acceleratorNative = accelerator.toNativeUtf8().cast<ffi.Char>();
    final handle = c.native_shortcut_manager_get_with_accelerator(
      acceleratorNative,
    );
    pkg_ffi.calloc.free(acceleratorNative);
    if (handle == 0) return null;
    return Shortcut.fromHandle(handle);
  }

  List<Shortcut> getAll() {
    final list = c.native_shortcut_manager_get_all();
    final items = <Shortcut>[];
    for (var i = 0; i < list.count; i++) {
      items.add(Shortcut.fromHandle(list.shortcuts[i]));
    }
    final listPointer = pkg_ffi.calloc<c.native_shortcut_list_t>();
    listPointer.ref = list;
    // The handles now belong to `items`; free just the array.
    c.native_shortcut_list_release(listPointer);
    pkg_ffi.calloc.free(listPointer);
    return items;
  }

  List<Shortcut> getByScope(ShortcutScope scope) {
    final list = c.native_shortcut_manager_get_by_scope(scope.raw);
    final items = <Shortcut>[];
    for (var i = 0; i < list.count; i++) {
      items.add(Shortcut.fromHandle(list.shortcuts[i]));
    }
    final listPointer = pkg_ffi.calloc<c.native_shortcut_list_t>();
    listPointer.ref = list;
    // The handles now belong to `items`; free just the array.
    c.native_shortcut_list_release(listPointer);
    pkg_ffi.calloc.free(listPointer);
    return items;
  }

  bool isAvailable(String accelerator) {
    final acceleratorNative = accelerator.toNativeUtf8().cast<ffi.Char>();
    final result = c.native_shortcut_manager_is_available(acceleratorNative);
    pkg_ffi.calloc.free(acceleratorNative);
    return result;
  }

  bool isValidAccelerator(String accelerator) {
    final acceleratorNative = accelerator.toNativeUtf8().cast<ffi.Char>();
    final result = c.native_shortcut_manager_is_valid_accelerator(
      acceleratorNative,
    );
    pkg_ffi.calloc.free(acceleratorNative);
    return result;
  }

  void setEnabled(bool enabled) {
    c.native_shortcut_manager_set_enabled(enabled);
  }

  bool isEnabled() {
    return c.native_shortcut_manager_is_enabled();
  }

  void emitShortcutActivated(ShortcutId id, String accelerator) {
    final acceleratorNative = accelerator.toNativeUtf8().cast<ffi.Char>();
    c.native_shortcut_manager_emit_shortcut_activated(id, acceleratorNative);
    pkg_ffi.calloc.free(acceleratorNative);
  }

  /// Registers [callback] for every `ShortcutEvent` this `ShortcutManager` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(FutureOr<void> Function(ShortcutEvent) callback) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<ffi.Void>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<ffi.Void> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : ShortcutEvent.fromNative(
                      event.cast<c.native_shortcut_event_t>().ref,
                    ),
              callback,
            ),
          );
        });
    return c.native_shortcut_manager_add_listener_async(
      callable.nativeFunction.cast(),
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_shortcut_manager_remove_listener(listenerId);
}
