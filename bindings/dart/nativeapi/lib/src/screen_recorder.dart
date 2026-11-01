// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'capture_source.dart';
import 'screen_capturer.dart';

import 'support.dart';

import 'callbacks.dart';

enum ScreenRecordingState {
  idle(0),
  starting(1),
  recording(2),
  stopping(3),
  completed(4),
  cancelled(5),
  failed(6);

  const ScreenRecordingState(this.value);
  final int value;

  static ScreenRecordingState fromValue(int value) => switch (value) {
    0 => ScreenRecordingState.idle,
    1 => ScreenRecordingState.starting,
    2 => ScreenRecordingState.recording,
    3 => ScreenRecordingState.stopping,
    4 => ScreenRecordingState.completed,
    5 => ScreenRecordingState.cancelled,
    6 => ScreenRecordingState.failed,
    _ => ScreenRecordingState.idle,
  };

  c.native_screen_recording_state_t get raw =>
      c.native_screen_recording_state_t.fromValue(value);
}

class ScreenRecordingOptions {
  const ScreenRecordingOptions({
    required this.capture,
    required this.frameRate,
    required this.videoBitrate,
  });

  final ScreenCaptureOptions capture;
  final double frameRate;
  final int videoBitrate;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is ScreenRecordingOptions &&
          other.capture == capture &&
          other.frameRate == frameRate &&
          other.videoBitrate == videoBitrate);

  @override
  int get hashCode => Object.hash(capture, frameRate, videoBitrate);

  @override
  String toString() =>
      'ScreenRecordingOptions(capture: $capture, frameRate: $frameRate, videoBitrate: $videoBitrate)';

  factory ScreenRecordingOptions.fromNative(
    c.native_screen_recording_options_t raw,
  ) => ScreenRecordingOptions(
    capture: ScreenCaptureOptions.fromNative(raw.capture),
    frameRate: raw.frame_rate,
    videoBitrate: raw.video_bitrate,
  );

  /// Allocates the C form; free it with [freeNative].
  ffi.Pointer<c.native_screen_recording_options_t> allocNative() {
    final pointer = pkg_ffi.calloc<c.native_screen_recording_options_t>();
    final capturePointer = capture.allocNative();
    pointer.ref.capture = capturePointer.ref;
    pkg_ffi.calloc.free(capturePointer);
    pointer.ref.frame_rate = frameRate;
    pointer.ref.video_bitrate = videoBitrate;
    return pointer;
  }

  static void freeNative(
    ffi.Pointer<c.native_screen_recording_options_t> pointer,
  ) {
    pkg_ffi.calloc.free(pointer);
  }
}

/// One `ScreenRecordingEvent`, in its concrete form.
sealed class ScreenRecordingEvent {
  const ScreenRecordingEvent();

  /// Reads the event out of its C form. Returns null for a variant this
  /// binding does not know about.
  static ScreenRecordingEvent? fromNative(
    c.native_screen_recording_event_t raw,
  ) {
    if (raw.typeAsInt ==
        c
            .native_screen_recording_event_type_t
            .NATIVE_SCREEN_RECORDING_EVENT_TYPE_STARTED
            .value) {
      return ScreenRecordingStartedEvent();
    }
    if (raw.typeAsInt ==
        c
            .native_screen_recording_event_type_t
            .NATIVE_SCREEN_RECORDING_EVENT_TYPE_COMPLETED
            .value) {
      return ScreenRecordingCompletedEvent(
        filePath: raw.data.completed.file_path == ffi.nullptr
            ? null
            : raw.data.completed.file_path.cast<pkg_ffi.Utf8>().toDartString(),
        duration: raw.data.completed.duration,
      );
    }
    if (raw.typeAsInt ==
        c
            .native_screen_recording_event_type_t
            .NATIVE_SCREEN_RECORDING_EVENT_TYPE_CANCELLED
            .value) {
      return ScreenRecordingCancelledEvent();
    }
    if (raw.typeAsInt ==
        c
            .native_screen_recording_event_type_t
            .NATIVE_SCREEN_RECORDING_EVENT_TYPE_FAILED
            .value) {
      return ScreenRecordingFailedEvent(
        message: raw.data.failed.message == ffi.nullptr
            ? null
            : raw.data.failed.message.cast<pkg_ffi.Utf8>().toDartString(),
      );
    }
    return null;
  }
}

final class ScreenRecordingStartedEvent extends ScreenRecordingEvent {
  const ScreenRecordingStartedEvent();
}

final class ScreenRecordingCompletedEvent extends ScreenRecordingEvent {
  const ScreenRecordingCompletedEvent({
    required this.filePath,
    required this.duration,
  });

  final String? filePath;
  final double duration;
}

final class ScreenRecordingCancelledEvent extends ScreenRecordingEvent {
  const ScreenRecordingCancelledEvent();
}

final class ScreenRecordingFailedEvent extends ScreenRecordingEvent {
  const ScreenRecordingFailedEvent({required this.message});

  final String? message;
}

class ScreenRecorder implements ffi.Finalizable {
  /// Adopts a handle returned by the C API and releases it when this
  /// object becomes unreachable.
  ScreenRecorder.fromHandle(this.nativeHandle) {
    _finalizer.attach(
      this,
      ffi.Pointer<ffi.Void>.fromAddress(nativeHandle),
      detach: this,
    );
  }

  /// Wraps a handle owned elsewhere; releasing it stays the owner's job.
  ScreenRecorder.borrowed(this.nativeHandle);

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
    c.native_screen_recorder_free(nativeHandle);
  }

  /// Creates a new `ScreenRecorder`; returns null if the native side failed.
  static ScreenRecorder? create() {
    final handle = c.native_screen_recorder_create();
    if (handle == 0) return null;
    return ScreenRecorder.fromHandle(handle);
  }

  static bool isSupported() {
    return c.native_screen_recorder_is_supported();
  }

  bool start(
    CaptureSource? source,
    String filePath,
    ScreenRecordingOptions options,
  ) {
    final filePathNative = filePath.toNativeUtf8().cast<ffi.Char>();
    final optionsPointer = options.allocNative();
    final result = c.native_screen_recorder_start(
      nativeHandle,
      source?.nativeHandle ?? 0,
      filePathNative,
      optionsPointer.ref,
    );
    pkg_ffi.calloc.free(filePathNative);
    ScreenRecordingOptions.freeNative(optionsPointer);
    return result;
  }

  bool stop() {
    return c.native_screen_recorder_stop(nativeHandle);
  }

  bool cancel() {
    return c.native_screen_recorder_cancel(nativeHandle);
  }

  ScreenRecordingState get state {
    final raw = c.native_screen_recorder_get_state(nativeHandle);
    return ScreenRecordingState.fromValue(raw.value);
  }

  bool get isActive {
    return c.native_screen_recorder_is_active(nativeHandle);
  }

  double get duration {
    return c.native_screen_recorder_get_duration(nativeHandle);
  }

  String? get filePath {
    final resultPointer = c.native_screen_recorder_get_file_path(nativeHandle);
    if (resultPointer == ffi.nullptr) return null;
    final result = resultPointer.cast<pkg_ffi.Utf8>().toDartString();
    c.free_c_str(resultPointer);
    return result;
  }

  /// Registers [callback] for every `ScreenRecordingEvent` this `ScreenRecorder` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(
    FutureOr<void> Function(ScreenRecordingEvent) callback,
  ) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<c.native_screen_recording_event_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<c.native_screen_recording_event_t> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : ScreenRecordingEvent.fromNative(event.ref),
              callback,
            ),
          );
        });
    return c.native_screen_recorder_add_listener_async(
      nativeHandle,
      callable.nativeFunction,
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_screen_recorder_remove_listener(nativeHandle, listenerId);
}
