// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dialog.dart';
import 'window.dart';
import 'unsupported.dart';

enum FileDialogMode {
  openFile(0),
  openFiles(1),
  saveFile(2),
  selectFolder(3);

  const FileDialogMode(this.value);
  final int value;

  static FileDialogMode fromValue(int value) => switch (value) {
    0 => FileDialogMode.openFile,
    1 => FileDialogMode.openFiles,
    2 => FileDialogMode.saveFile,
    3 => FileDialogMode.selectFolder,
    _ => FileDialogMode.openFile,
  };
}

enum FileDialogResult {
  none(0),
  accepted(1),
  cancelled(2),
  failed(3);

  const FileDialogResult(this.value);
  final int value;

  static FileDialogResult fromValue(int value) => switch (value) {
    0 => FileDialogResult.none,
    1 => FileDialogResult.accepted,
    2 => FileDialogResult.cancelled,
    3 => FileDialogResult.failed,
    _ => FileDialogResult.none,
  };
}

class FileDialog {
  FileDialog.fromHandle(this.nativeHandle);
  FileDialog.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  static FileDialog? create(FileDialogMode mode) => unsupported();

  static bool isSupported() => unsupported();

  bool setParentWindow(Window? window) => unsupported();

  bool setFileTypes(List<String> extensions) => unsupported();

  bool setSuggestedFileName(String name) => unsupported();

  DialogModality get modality => unsupported();

  set modality(DialogModality value) => unsupported();

  bool open() => unsupported();

  bool close() => unsupported();

  FileDialogResult get result => unsupported();

  List<String> get paths => unsupported();

  String? get lastError => unsupported();
}
