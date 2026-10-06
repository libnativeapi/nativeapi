// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/color.dart';
import 'foundation/geometry.dart';
import 'image.dart';
import 'window.dart';
import 'support.dart';
import 'unsupported.dart';

typedef ViewId = int;

enum ViewLayout {
  absolute(0),
  row(1),
  column(2);

  const ViewLayout(this.value);
  final int value;

  static ViewLayout fromValue(int value) => switch (value) {
    0 => ViewLayout.absolute,
    1 => ViewLayout.row,
    2 => ViewLayout.column,
    _ => ViewLayout.absolute,
  };
}

enum ViewAlignment {
  stretch(0),
  start(1),
  center(2),
  end(3);

  const ViewAlignment(this.value);
  final int value;

  static ViewAlignment fromValue(int value) => switch (value) {
    0 => ViewAlignment.stretch,
    1 => ViewAlignment.start,
    2 => ViewAlignment.center,
    3 => ViewAlignment.end,
    _ => ViewAlignment.stretch,
  };
}

enum TextAlignment {
  start(0),
  center(1),
  end(2);

  const TextAlignment(this.value);
  final int value;

  static TextAlignment fromValue(int value) => switch (value) {
    0 => TextAlignment.start,
    1 => TextAlignment.center,
    2 => TextAlignment.end,
    _ => TextAlignment.start,
  };
}

enum ViewBackend {
  native(0),
  winUi3(1);

  const ViewBackend(this.value);
  final int value;

  static ViewBackend fromValue(int value) => switch (value) {
    0 => ViewBackend.native,
    1 => ViewBackend.winUi3,
    _ => ViewBackend.native,
  };
}

/// One `ViewEvent`, in its concrete form.
sealed class ViewEvent {
  const ViewEvent();

  ViewId get viewId;
}

final class ViewFocusedEvent extends ViewEvent {
  const ViewFocusedEvent({required this.viewId});

  @override
  final ViewId viewId;
}

final class ViewBlurredEvent extends ViewEvent {
  const ViewBlurredEvent({required this.viewId});

  @override
  final ViewId viewId;
}

final class ButtonClickedEvent extends ViewEvent {
  const ButtonClickedEvent({required this.viewId});

  @override
  final ViewId viewId;
}

final class TextFieldChangedEvent extends ViewEvent {
  const TextFieldChangedEvent({required this.viewId, required this.text});

  @override
  final ViewId viewId;
  final String? text;
}

final class TextFieldSubmittedEvent extends ViewEvent {
  const TextFieldSubmittedEvent({required this.viewId});

  @override
  final ViewId viewId;
}

class View {
  View.fromHandle(this.nativeHandle);
  View.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static View? create() => unsupported();

  static bool isSupported() => unsupported();

  static bool isBackendSupported(ViewBackend backend) => unsupported();

  static bool setDefaultBackend(ViewBackend backend) => unsupported();

  static ViewBackend getDefaultBackend() => unsupported();

  ViewId get id => unsupported();

  ViewBackend get backend => unsupported();

  void addSubview(View? subview) => unsupported();

  void insertSubview(int index, View? subview) => unsupported();

  bool removeSubview(View? subview) => unsupported();

  bool removeSubviewAt(int index) => unsupported();

  void clearSubviews() => unsupported();

  int get subviewCount => unsupported();

  View? getSubviewAt(int index) => unsupported();

  List<View> get subviews => unsupported();

  View? get parent => unsupported();

  Window? get window => unsupported();

  set frame(Rectangle value) => unsupported();

  Rectangle get frame => unsupported();

  set preferredSize(Size value) => unsupported();

  Size get preferredSize => unsupported();

  Size get intrinsicSize => unsupported();

  set flex(double value) => unsupported();

  double get flex => unsupported();

  set alignment(ViewAlignment value) => unsupported();

  ViewAlignment get alignment => unsupported();

  set layout(ViewLayout value) => unsupported();

  ViewLayout get layout => unsupported();

  set spacing(double value) => unsupported();

  double get spacing => unsupported();

  set padding(EdgeInsets value) => unsupported();

  EdgeInsets get padding => unsupported();

  set isVisible(bool value) => unsupported();

  bool get isVisible => unsupported();

  set isEnabled(bool value) => unsupported();

  bool get isEnabled => unsupported();

  set backgroundColor(Color value) => unsupported();

  Color get backgroundColor => unsupported();

  set tooltip(String? value) => unsupported();

  String? get tooltip => unsupported();

  void focus() => unsupported();

  void blur() => unsupported();

  bool get isFocused => unsupported();

  ListenerId addListener(FutureOr<void> Function(ViewEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}

class Label extends View {
  Label.fromHandle(super.nativeHandle) : super.fromHandle();
  Label.borrowed(super.nativeHandle) : super.borrowed();

  static Label? create(String text) => unsupported();

  set text(String value) => unsupported();

  String? get text => unsupported();

  set textColor(Color value) => unsupported();

  Color get textColor => unsupported();

  set fontSize(double value) => unsupported();

  double get fontSize => unsupported();

  set textAlignment(TextAlignment value) => unsupported();

  TextAlignment get textAlignment => unsupported();
}

class Button extends View {
  Button.fromHandle(super.nativeHandle) : super.fromHandle();
  Button.borrowed(super.nativeHandle) : super.borrowed();

  static Button? create(String text) => unsupported();

  set text(String value) => unsupported();

  String? get text => unsupported();
}

class TextField extends View {
  TextField.fromHandle(super.nativeHandle) : super.fromHandle();
  TextField.borrowed(super.nativeHandle) : super.borrowed();

  static TextField? create(String text) => unsupported();

  set text(String value) => unsupported();

  String? get text => unsupported();

  set textColor(Color value) => unsupported();

  Color get textColor => unsupported();

  set fontSize(double value) => unsupported();

  double get fontSize => unsupported();

  set textAlignment(TextAlignment value) => unsupported();

  TextAlignment get textAlignment => unsupported();

  set placeholder(String? value) => unsupported();

  String? get placeholder => unsupported();

  set isEditable(bool value) => unsupported();

  bool get isEditable => unsupported();

  set isSecure(bool value) => unsupported();

  bool get isSecure => unsupported();

  set isMultiline(bool value) => unsupported();

  bool get isMultiline => unsupported();
}

class ImageView extends View {
  ImageView.fromHandle(super.nativeHandle) : super.fromHandle();
  ImageView.borrowed(super.nativeHandle) : super.borrowed();

  static ImageView? create() => unsupported();

  set image(Image? value) => unsupported();

  Image? get image => unsupported();
}
