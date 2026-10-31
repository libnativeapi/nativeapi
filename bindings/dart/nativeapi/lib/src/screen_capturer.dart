// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'capture_source.dart';
import 'foundation/geometry.dart';
import 'image.dart';
import 'window.dart';

import 'callbacks.dart';

class CaptureSourceSelectionOptions {
  const CaptureSourceSelectionOptions({
    required this.allowDisplays,
    required this.allowWindows,
    this.parentWindow,
  });

  final bool allowDisplays;
  final bool allowWindows;
  final Window? parentWindow;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is CaptureSourceSelectionOptions &&
          other.allowDisplays == allowDisplays &&
          other.allowWindows == allowWindows &&
          other.parentWindow == parentWindow);

  @override
  int get hashCode => Object.hash(allowDisplays, allowWindows, parentWindow);

  @override
  String toString() =>
      'CaptureSourceSelectionOptions(allowDisplays: $allowDisplays, allowWindows: $allowWindows, parentWindow: $parentWindow)';

  factory CaptureSourceSelectionOptions.fromNative(
    c.native_capture_source_selection_options_t raw,
  ) => CaptureSourceSelectionOptions(
    allowDisplays: raw.allow_displays,
    allowWindows: raw.allow_windows,
    parentWindow: raw.parent_window == 0
        ? null
        : Window.fromHandle(c.native_handle_retain(raw.parent_window)),
  );

  /// Allocates the C form; free it with [freeNative].
  ffi.Pointer<c.native_capture_source_selection_options_t> allocNative() {
    final pointer = pkg_ffi
        .calloc<c.native_capture_source_selection_options_t>();
    pointer.ref.allow_displays = allowDisplays;
    pointer.ref.allow_windows = allowWindows;
    pointer.ref.parent_window = parentWindow?.nativeHandle ?? 0;
    return pointer;
  }

  static void freeNative(
    ffi.Pointer<c.native_capture_source_selection_options_t> pointer,
  ) {
    pkg_ffi.calloc.free(pointer);
  }
}

class ScreenCaptureOptions {
  const ScreenCaptureOptions({
    required this.includeCursor,
    required this.cropEnabled,
    required this.crop,
  });

  final bool includeCursor;
  final bool cropEnabled;
  final Rectangle crop;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is ScreenCaptureOptions &&
          other.includeCursor == includeCursor &&
          other.cropEnabled == cropEnabled &&
          other.crop == crop);

  @override
  int get hashCode => Object.hash(includeCursor, cropEnabled, crop);

  @override
  String toString() =>
      'ScreenCaptureOptions(includeCursor: $includeCursor, cropEnabled: $cropEnabled, crop: $crop)';

  factory ScreenCaptureOptions.fromNative(
    c.native_screen_capture_options_t raw,
  ) => ScreenCaptureOptions(
    includeCursor: raw.include_cursor,
    cropEnabled: raw.crop_enabled,
    crop: Rectangle.fromNative(raw.crop),
  );

  /// Allocates the C form; free it with [freeNative].
  ffi.Pointer<c.native_screen_capture_options_t> allocNative() {
    final pointer = pkg_ffi.calloc<c.native_screen_capture_options_t>();
    pointer.ref.include_cursor = includeCursor;
    pointer.ref.crop_enabled = cropEnabled;
    final cropPointer = crop.allocNative();
    pointer.ref.crop = cropPointer.ref;
    pkg_ffi.calloc.free(cropPointer);
    return pointer;
  }

  static void freeNative(
    ffi.Pointer<c.native_screen_capture_options_t> pointer,
  ) {
    pkg_ffi.calloc.free(pointer);
  }
}

class ScreenCapturer {
  ScreenCapturer._();
  static bool isSupported() {
    return c.native_screen_capturer_is_supported();
  }

  static bool isSourceSelectionSupported() {
    return c.native_screen_capturer_is_source_selection_supported();
  }

  static bool isDirectCaptureSupported(CaptureSourceType type) {
    return c.native_screen_capturer_is_direct_capture_supported(type.raw);
  }

  static void selectSource(
    CaptureSourceSelectionOptions options,
    void Function(bool, CaptureSource?) callback,
  ) {
    final optionsPointer = options.allocNative();
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Bool,
            ffi.Uint64,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          bool arg0,
          int arg1,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          try {
            if (c.native_event_delivery_is_active(delivery)) {
              callback(
                arg0,
                arg1 == 0
                    ? null
                    : CaptureSource.fromHandle(c.native_handle_retain(arg1)),
              );
            }
          } finally {
            c.native_event_delivery_complete(delivery, true);
          }
        });
    c.native_screen_capturer_select_source(
      optionsPointer.ref,
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
    CaptureSourceSelectionOptions.freeNative(optionsPointer);
  }

  static void capture(
    CaptureSource? source,
    ScreenCaptureOptions options,
    void Function(bool, Image?) callback,
  ) {
    final optionsPointer = options.allocNative();
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Bool,
            ffi.Uint64,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          bool arg0,
          int arg1,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          try {
            if (c.native_event_delivery_is_active(delivery)) {
              callback(
                arg0,
                arg1 == 0
                    ? null
                    : Image.fromHandle(c.native_handle_retain(arg1)),
              );
            }
          } finally {
            c.native_event_delivery_complete(delivery, true);
          }
        });
    c.native_screen_capturer_capture(
      source?.nativeHandle ?? 0,
      optionsPointer.ref,
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
    ScreenCaptureOptions.freeNative(optionsPointer);
  }
}
