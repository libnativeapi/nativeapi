// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'support.dart';

import 'callbacks.dart';

/// One `ProtocolHandlerEvent`, in its concrete form.
sealed class ProtocolHandlerEvent {
  const ProtocolHandlerEvent();

  /// Reads the event out of its C form. Returns null for a variant this
  /// binding does not know about.
  static ProtocolHandlerEvent? fromNative(
    c.native_protocol_handler_event_t raw,
  ) {
    if (raw.typeAsInt ==
        c
            .native_protocol_handler_event_type_t
            .NATIVE_PROTOCOL_HANDLER_EVENT_TYPE_PROTOCOL_URL_RECEIVED
            .value) {
      return ProtocolUrlReceivedEvent(
        url: raw.data.protocol_url_received.url == ffi.nullptr
            ? null
            : raw.data.protocol_url_received.url
                  .cast<pkg_ffi.Utf8>()
                  .toDartString(),
      );
    }
    return null;
  }
}

final class ProtocolUrlReceivedEvent extends ProtocolHandlerEvent {
  const ProtocolUrlReceivedEvent({required this.url});

  final String? url;
}

class ProtocolHandler {
  const ProtocolHandler._();

  /// The shared instance backed by the native singleton.
  static const ProtocolHandler instance = ProtocolHandler._();

  bool isSupported() {
    return c.native_protocol_handler_is_supported();
  }

  bool register(String scheme) {
    final schemeNative = scheme.toNativeUtf8().cast<ffi.Char>();
    final result = c.native_protocol_handler_register(schemeNative);
    pkg_ffi.calloc.free(schemeNative);
    return result;
  }

  String? getInitialUrl() {
    final resultPointer = c.native_protocol_handler_get_initial_url();
    if (resultPointer == ffi.nullptr) return null;
    final result = resultPointer.cast<pkg_ffi.Utf8>().toDartString();
    c.free_c_str(resultPointer);
    return result;
  }

  /// Registers [callback] for every `ProtocolHandlerEvent` this `ProtocolHandler` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(
    FutureOr<void> Function(ProtocolHandlerEvent) callback,
  ) {
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
                  : ProtocolHandlerEvent.fromNative(
                      event.cast<c.native_protocol_handler_event_t>().ref,
                    ),
              callback,
            ),
          );
        });
    return c.native_protocol_handler_add_listener_async(
      callable.nativeFunction.cast(),
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_protocol_handler_remove_listener(listenerId);
}
