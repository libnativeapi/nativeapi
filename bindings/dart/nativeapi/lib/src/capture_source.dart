// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'display.dart';
import 'window.dart';

enum CaptureSourceType {
  none(0),
  display(1),
  window(2),
  unknown(3);

  const CaptureSourceType(this.value);
  final int value;

  static CaptureSourceType fromValue(int value) => switch (value) {
    0 => CaptureSourceType.none,
    1 => CaptureSourceType.display,
    2 => CaptureSourceType.window,
    3 => CaptureSourceType.unknown,
    _ => CaptureSourceType.none,
  };

  c.native_capture_source_type_t get raw =>
      c.native_capture_source_type_t.fromValue(value);
}

class CaptureSource implements ffi.Finalizable {
  /// Adopts a handle returned by the C API and releases it when this
  /// object becomes unreachable.
  CaptureSource.fromHandle(this.nativeHandle) {
    _finalizer.attach(
      this,
      ffi.Pointer<ffi.Void>.fromAddress(nativeHandle),
      detach: this,
    );
  }

  /// Wraps a handle owned elsewhere; releasing it stays the owner's job.
  CaptureSource.borrowed(this.nativeHandle);

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
    c.native_capture_source_free(nativeHandle);
  }

  static CaptureSource? fromDisplay(Display? display) {
    final handle = c.native_capture_source_from_display(
      display?.nativeHandle ?? 0,
    );
    if (handle == 0) return null;
    return CaptureSource.fromHandle(handle);
  }

  static CaptureSource? fromWindow(Window? window) {
    final handle = c.native_capture_source_from_window(
      window?.nativeHandle ?? 0,
    );
    if (handle == 0) return null;
    return CaptureSource.fromHandle(handle);
  }

  CaptureSourceType get type {
    final raw = c.native_capture_source_get_type(nativeHandle);
    return CaptureSourceType.fromValue(raw.value);
  }

  String? get name {
    final resultPointer = c.native_capture_source_get_name(nativeHandle);
    if (resultPointer == ffi.nullptr) return null;
    final result = resultPointer.cast<pkg_ffi.Utf8>().toDartString();
    c.free_c_str(resultPointer);
    return result;
  }
}
