// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'capture_source.dart';
import 'screen_capturer.dart';
import 'support.dart';
import 'unsupported.dart';

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
}

/// One `ScreenRecordingEvent`, in its concrete form.
sealed class ScreenRecordingEvent {
  const ScreenRecordingEvent();
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

class ScreenRecorder {
  ScreenRecorder.fromHandle(this.nativeHandle);
  ScreenRecorder.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static ScreenRecorder? create() => unsupported();

  static bool isSupported() => unsupported();

  bool start(
    CaptureSource? source,
    String filePath,
    ScreenRecordingOptions options,
  ) => unsupported();

  bool stop() => unsupported();

  bool cancel() => unsupported();

  ScreenRecordingState get state => unsupported();

  bool get isActive => unsupported();

  double get duration => unsupported();

  String? get filePath => unsupported();

  ListenerId addListener(
    FutureOr<void> Function(ScreenRecordingEvent) callback,
  ) => unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
