// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/color.dart';
import 'foundation/event_request.dart';
import 'foundation/geometry.dart';
import 'view.dart';
import 'window_shadow.dart';
import 'window_shape.dart';
import 'support.dart';
import 'unsupported.dart';

typedef WindowId = int;

enum TitleBarStyle {
  normal(0),
  hidden(1);

  const TitleBarStyle(this.value);
  final int value;

  static TitleBarStyle fromValue(int value) => switch (value) {
    0 => TitleBarStyle.normal,
    1 => TitleBarStyle.hidden,
    _ => TitleBarStyle.normal,
  };
}

enum WindowCornerPreference {
  default_(0),
  doNotRound(1),
  round(2),
  roundSmall(3);

  const WindowCornerPreference(this.value);
  final int value;

  static WindowCornerPreference fromValue(int value) => switch (value) {
    0 => WindowCornerPreference.default_,
    1 => WindowCornerPreference.doNotRound,
    2 => WindowCornerPreference.round,
    3 => WindowCornerPreference.roundSmall,
    _ => WindowCornerPreference.default_,
  };
}

enum WindowProperty {
  title(0),
  resizable(1),
  movable(2),
  minimizable(3),
  maximizable(4),
  fullScreenable(5),
  closable(6),
  windowControlButtonsVisible(7),
  alwaysOnTop(8),
  alwaysOnBottom(9),
  titleBarStyle(10);

  const WindowProperty(this.value);
  final int value;

  static WindowProperty fromValue(int value) => switch (value) {
    0 => WindowProperty.title,
    1 => WindowProperty.resizable,
    2 => WindowProperty.movable,
    3 => WindowProperty.minimizable,
    4 => WindowProperty.maximizable,
    5 => WindowProperty.fullScreenable,
    6 => WindowProperty.closable,
    7 => WindowProperty.windowControlButtonsVisible,
    8 => WindowProperty.alwaysOnTop,
    9 => WindowProperty.alwaysOnBottom,
    10 => WindowProperty.titleBarStyle,
    _ => WindowProperty.title,
  };
}

enum WindowOcclusionState {
  unknown(0),
  visible(1),
  occluded(2);

  const WindowOcclusionState(this.value);
  final int value;

  static WindowOcclusionState fromValue(int value) => switch (value) {
    0 => WindowOcclusionState.unknown,
    1 => WindowOcclusionState.visible,
    2 => WindowOcclusionState.occluded,
    _ => WindowOcclusionState.unknown,
  };
}

enum VisualEffect {
  none(0),
  blur(1),
  acrylic(2),
  mica(3),
  micaAlt(4),
  hud(5),
  popover(6),
  menu(7);

  const VisualEffect(this.value);
  final int value;

  static VisualEffect fromValue(int value) => switch (value) {
    0 => VisualEffect.none,
    1 => VisualEffect.blur,
    2 => VisualEffect.acrylic,
    3 => VisualEffect.mica,
    4 => VisualEffect.micaAlt,
    5 => VisualEffect.hud,
    6 => VisualEffect.popover,
    7 => VisualEffect.menu,
    _ => VisualEffect.none,
  };
}

enum ResizeEdge {
  top(0),
  left(1),
  right(2),
  bottom(3),
  topLeft(4),
  topRight(5),
  bottomLeft(6),
  bottomRight(7);

  const ResizeEdge(this.value);
  final int value;

  static ResizeEdge fromValue(int value) => switch (value) {
    0 => ResizeEdge.top,
    1 => ResizeEdge.left,
    2 => ResizeEdge.right,
    3 => ResizeEdge.bottom,
    4 => ResizeEdge.topLeft,
    5 => ResizeEdge.topRight,
    6 => ResizeEdge.bottomLeft,
    7 => ResizeEdge.bottomRight,
    _ => ResizeEdge.top,
  };
}

/// One `WindowEvent`, in its concrete form.
sealed class WindowEvent {
  const WindowEvent();

  WindowId get windowId;
}

final class WindowFocusedEvent extends WindowEvent {
  const WindowFocusedEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowBlurredEvent extends WindowEvent {
  const WindowBlurredEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowMinimizedEvent extends WindowEvent {
  const WindowMinimizedEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowMaximizedEvent extends WindowEvent {
  const WindowMaximizedEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowRestoredEvent extends WindowEvent {
  const WindowRestoredEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowMovedEvent extends WindowEvent {
  const WindowMovedEvent({required this.windowId, required this.newPosition});

  @override
  final WindowId windowId;
  final Point newPosition;
}

final class WindowResizedEvent extends WindowEvent {
  const WindowResizedEvent({required this.windowId, required this.newSize});

  @override
  final WindowId windowId;
  final Size newSize;
}

final class WindowCreatedEvent extends WindowEvent {
  const WindowCreatedEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowClosedEvent extends WindowEvent {
  const WindowClosedEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowEnteredFullScreenEvent extends WindowEvent {
  const WindowEnteredFullScreenEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowExitedFullScreenEvent extends WindowEvent {
  const WindowExitedFullScreenEvent({required this.windowId});

  @override
  final WindowId windowId;
}

final class WindowCloseRequestedEvent extends WindowEvent {
  const WindowCloseRequestedEvent({
    required this.windowId,
    required this.request,
  });

  @override
  final WindowId windowId;
  final EventRequest request;
}

final class WindowPropertyChangedEvent extends WindowEvent {
  const WindowPropertyChangedEvent({
    required this.windowId,
    required this.property,
  });

  @override
  final WindowId windowId;
  final WindowProperty property;
}

final class WindowOcclusionChangedEvent extends WindowEvent {
  const WindowOcclusionChangedEvent({
    required this.windowId,
    required this.occlusionState,
  });

  @override
  final WindowId windowId;
  final WindowOcclusionState occlusionState;
}

class Window {
  Window.fromHandle(this.nativeHandle);
  Window.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static Window? create() => unsupported();

  static bool isCloseSupported() => unsupported();

  bool close() => unsupported();

  WindowId get id => unsupported();

  View? get contentView => unsupported();

  void focus() => unsupported();

  void blur() => unsupported();

  bool get isFocused => unsupported();

  void show() => unsupported();

  void showInactive() => unsupported();

  void hide() => unsupported();

  bool get isVisible => unsupported();

  WindowOcclusionState get occlusionState => unsupported();

  static bool isOcclusionStateSupported() => unsupported();

  void maximize() => unsupported();

  void unmaximize() => unsupported();

  bool get isMaximized => unsupported();

  void minimize() => unsupported();

  void restore() => unsupported();

  bool get isMinimized => unsupported();

  set isFullScreen(bool value) => unsupported();

  bool get isFullScreen => unsupported();

  set bounds(Rectangle value) => unsupported();

  Rectangle get bounds => unsupported();

  set contentBounds(Rectangle value) => unsupported();

  Rectangle get contentBounds => unsupported();

  void setSize(Size size, bool animate) => unsupported();

  Size get size => unsupported();

  set contentSize(Size value) => unsupported();

  Size get contentSize => unsupported();

  set minimumSize(Size value) => unsupported();

  Size get minimumSize => unsupported();

  set maximumSize(Size value) => unsupported();

  Size get maximumSize => unsupported();

  set aspectRatio(double value) => unsupported();

  double get aspectRatio => unsupported();

  set isResizable(bool value) => unsupported();

  bool get isResizable => unsupported();

  set isMovable(bool value) => unsupported();

  bool get isMovable => unsupported();

  set isMinimizable(bool value) => unsupported();

  bool get isMinimizable => unsupported();

  set isMaximizable(bool value) => unsupported();

  bool get isMaximizable => unsupported();

  set isFullScreenable(bool value) => unsupported();

  bool get isFullScreenable => unsupported();

  set isClosable(bool value) => unsupported();

  bool get isClosable => unsupported();

  set isWindowControlButtonsVisible(bool value) => unsupported();

  bool get isWindowControlButtonsVisible => unsupported();

  set isAlwaysOnTop(bool value) => unsupported();

  bool get isAlwaysOnTop => unsupported();

  set isAlwaysOnBottom(bool value) => unsupported();

  bool get isAlwaysOnBottom => unsupported();

  bool setParentWindow(Window? parent) => unsupported();

  Window? get parentWindow => unsupported();

  set isNonActivating(bool value) => unsupported();

  bool get isNonActivating => unsupported();

  set position(Point value) => unsupported();

  Point get position => unsupported();

  void center() => unsupported();

  set title(String value) => unsupported();

  String? get title => unsupported();

  bool setTitleBarColors(Color background, Color foreground) => unsupported();

  bool resetTitleBarColors() => unsupported();

  set titleBarStyle(TitleBarStyle value) => unsupported();

  TitleBarStyle get titleBarStyle => unsupported();

  bool setCornerPreference(WindowCornerPreference preference) => unsupported();

  WindowCornerPreference get cornerPreference => unsupported();

  static bool isCornerPreferenceSupported() => unsupported();

  bool setContentUnderTitleBar(bool isContentUnderTitleBar) => unsupported();

  bool get isContentUnderTitleBar => unsupported();

  static bool isContentUnderTitleBarSupported() => unsupported();

  bool setContentProtection(bool isContentProtected) => unsupported();

  bool get isContentProtected => unsupported();

  static bool isContentProtectionSupported() => unsupported();

  set hasShadow(bool value) => unsupported();

  bool get hasShadow => unsupported();

  bool setCustomShadow(WindowShadow? shadow) => unsupported();

  WindowShadow? get customShadow => unsupported();

  set opacity(double value) => unsupported();

  double get opacity => unsupported();

  bool setVisualEffect(VisualEffect effect) => unsupported();

  VisualEffect get visualEffect => unsupported();

  static bool isVisualEffectSupported(VisualEffect effect) => unsupported();

  bool setShape(WindowShape? shape) => unsupported();

  bool get isShaped => unsupported();

  static bool isShapeSupported() => unsupported();

  bool setInputShape(WindowShape? shape) => unsupported();

  bool get isInputShaped => unsupported();

  static bool isInputShapeSupported() => unsupported();

  set backgroundColor(Color value) => unsupported();

  Color get backgroundColor => unsupported();

  set isVisibleOnAllWorkspaces(bool value) => unsupported();

  bool get isVisibleOnAllWorkspaces => unsupported();

  set isVisibleInTaskbar(bool value) => unsupported();

  bool get isVisibleInTaskbar => unsupported();

  bool setIgnoreMouseEvents(bool isIgnoreMouseEvents, bool forward) =>
      unsupported();

  bool get isIgnoreMouseEvents => unsupported();

  bool get isMouseMoveForwardingEnabled => unsupported();

  static bool isMouseMoveForwardingSupported() => unsupported();

  set isFocusable(bool value) => unsupported();

  bool get isFocusable => unsupported();

  bool showSystemMenu(Point position) => unsupported();

  static bool isSystemMenuSupported() => unsupported();

  bool performTitleBarDoubleClick() => unsupported();

  void startDragging() => unsupported();

  void startResizing(ResizeEdge edge) => unsupported();

  ListenerId addListener(FutureOr<void> Function(WindowEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
