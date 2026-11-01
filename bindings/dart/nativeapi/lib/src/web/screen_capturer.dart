// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'capture_source.dart';
import 'foundation/geometry.dart';
import 'image.dart';
import 'window.dart';
import 'unsupported.dart';

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
}

class ScreenCapturer {
  const ScreenCapturer._();

  static const ScreenCapturer instance = ScreenCapturer._();

  static bool isSupported() => unsupported();

  static bool isSourceSelectionSupported() => unsupported();

  static bool isDirectCaptureSupported(CaptureSourceType type) => unsupported();

  static void selectSource(
    CaptureSourceSelectionOptions options,
    void Function(bool, CaptureSource?) callback,
  ) => unsupported();

  static void capture(
    CaptureSource? source,
    ScreenCaptureOptions options,
    void Function(bool, Image?) callback,
  ) => unsupported();
}
