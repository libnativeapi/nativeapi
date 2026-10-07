// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

class EventDecision implements ffi.Finalizable {
  /// Adopts a handle returned by the C API and releases it when this
  /// object becomes unreachable.
  EventDecision.fromHandle(this.nativeHandle) {
    _finalizer.attach(
      this,
      ffi.Pointer<ffi.Void>.fromAddress(nativeHandle),
      detach: this,
    );
  }

  /// Wraps a handle owned elsewhere; releasing it stays the owner's job.
  EventDecision.borrowed(this.nativeHandle);

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
    c.native_event_decision_free(nativeHandle);
  }

  bool accept() {
    return c.native_event_decision_accept(nativeHandle);
  }

  bool cancel() {
    return c.native_event_decision_cancel(nativeHandle);
  }

  bool get isPending {
    return c.native_event_decision_is_pending(nativeHandle);
  }
}

class EventRequest implements ffi.Finalizable {
  /// Adopts a handle returned by the C API and releases it when this
  /// object becomes unreachable.
  EventRequest.fromHandle(this.nativeHandle) {
    _finalizer.attach(
      this,
      ffi.Pointer<ffi.Void>.fromAddress(nativeHandle),
      detach: this,
    );
  }

  /// Wraps a handle owned elsewhere; releasing it stays the owner's job.
  EventRequest.borrowed(this.nativeHandle);

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
    c.native_event_request_free(nativeHandle);
  }

  bool get isCancelable {
    return c.native_event_request_is_cancelable(nativeHandle);
  }

  bool get isCancelled {
    return c.native_event_request_is_cancelled(nativeHandle);
  }

  bool get isPending {
    return c.native_event_request_is_pending(nativeHandle);
  }

  bool cancel() {
    return c.native_event_request_cancel(nativeHandle);
  }

  EventDecision? defer() {
    final handle = c.native_event_request_defer(nativeHandle);
    if (handle == 0) return null;
    return EventDecision.fromHandle(handle);
  }
}
