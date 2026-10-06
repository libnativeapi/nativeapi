import 'dart:ui' show Offset;

import 'package:nativeapi/nativeapi.dart' show Window;

import '../conversions.dart';

extension NativeWindowGeometry on Window {
  /// Offset from the window's outer frame to its content area (title bar and
  /// left border), in logical pixels.
  ///
  /// `WindowDragSession` anchors on the frame while Flutter reports positions
  /// inside the content, so an anchor derived from a pointer position needs
  /// this added.
  Offset get contentInset =>
      contentBounds.toRect().topLeft - bounds.toRect().topLeft;
}
