// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dialog.dart';
import 'window.dart';
import 'unsupported.dart';

enum MessageDialogResult {
  none(0),
  primary(1),
  secondary(2),
  close(3);

  const MessageDialogResult(this.value);
  final int value;

  static MessageDialogResult fromValue(int value) => switch (value) {
    0 => MessageDialogResult.none,
    1 => MessageDialogResult.primary,
    2 => MessageDialogResult.secondary,
    3 => MessageDialogResult.close,
    _ => MessageDialogResult.none,
  };
}

class MessageDialog {
  MessageDialog.fromHandle(this.nativeHandle);
  MessageDialog.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static MessageDialog? create(String title, String message) => unsupported();

  static bool isExtendedSupported() => unsupported();

  bool setButtons(String primary, String secondary, String close) =>
      unsupported();

  bool setDefaultButton(MessageDialogResult button) => unsupported();

  bool setParentWindow(Window? window) => unsupported();

  MessageDialogResult get result => unsupported();

  bool get isOpen => unsupported();

  bool setInputEnabled(bool enabled) => unsupported();

  bool setInputText(String text) => unsupported();

  String? get inputText => unsupported();

  bool setCheckbox(String label, bool checked) => unsupported();

  bool get isCheckboxChecked => unsupported();

  bool setProgress(double value) => unsupported();

  set title(String value) => unsupported();

  String? get title => unsupported();

  set message(String value) => unsupported();

  String? get message => unsupported();

  DialogModality get modality => unsupported();

  set modality(DialogModality value) => unsupported();

  bool open() => unsupported();

  bool close() => unsupported();
}
