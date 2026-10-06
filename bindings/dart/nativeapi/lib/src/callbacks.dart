// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;

/// Owns every NativeCallable handed to the C API until the core releases it.
abstract final class NativeCallbacks {
  static final _callables = <int, ffi.NativeCallable<Function>>{};
  static var _nextToken = 0;

  /// Keeps borrowed event handles valid across the callback's asynchronous work.
  /// Removal suppresses callbacks that have not begun, without closing their
  /// NativeCallable until every delivery has released its payload.
  static Future<void> deliverEvent<T>(
    int delivery,
    T? Function() convert,
    FutureOr<void> Function(T) callback,
  ) async {
    var accept = false;
    try {
      if (!c.native_event_delivery_is_active(delivery)) return;
      final value = convert();
      if (value == null) return;
      await callback(value);
      accept = true;
    } finally {
      c.native_event_delivery_complete(delivery, accept);
    }
  }

  /// The user_data to pass with [callable]: a token the release maps back to
  /// it. Null for a null callable, which has nothing to release.
  static ffi.Pointer<ffi.Void> userData(
    ffi.NativeCallable<Function>? callable,
  ) {
    if (callable == null) return ffi.nullptr;
    final token = ++_nextToken;
    _callables[token] = callable;
    return ffi.Pointer<ffi.Void>.fromAddress(token);
  }

  /// The release function to pass with every callback. The core may call it
  /// from any thread, so it is a listener: the callable is closed on this
  /// isolate, after the native side has already let go of it.
  static ffi.Pointer<
    ffi.NativeFunction<ffi.Void Function(ffi.Pointer<ffi.Void>)>
  >
  get release => _release.nativeFunction;

  static final _release =
      ffi.NativeCallable<ffi.Void Function(ffi.Pointer<ffi.Void>)>.listener((
        ffi.Pointer<ffi.Void> userData,
      ) {
        _callables.remove(userData.address)?.close();
      })..keepIsolateAlive = false;
}
