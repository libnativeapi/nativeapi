// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'image.dart';
import 'support.dart';
import 'unsupported.dart';

class ClipboardData {
  const ClipboardData({
    this.text,
    this.html,
    this.image,
    this.filePaths = const [],
  });

  final String? text;
  final String? html;
  final Image? image;
  final List<String> filePaths;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is ClipboardData &&
          other.text == text &&
          other.html == html &&
          other.image == image &&
          _listEquals(other.filePaths, filePaths));

  @override
  int get hashCode => Object.hash(text, html, image, Object.hashAll(filePaths));

  @override
  String toString() =>
      'ClipboardData(text: $text, html: $html, image: $image, filePaths: $filePaths)';

  static bool _listEquals<T>(List<T> a, List<T> b) {
    if (a.length != b.length) return false;
    for (var i = 0; i < a.length; i++) {
      if (a[i] != b[i]) return false;
    }
    return true;
  }
}

/// One `ClipboardEvent`, in its concrete form.
sealed class ClipboardEvent {
  const ClipboardEvent();
}

final class ClipboardChangedEvent extends ClipboardEvent {
  const ClipboardChangedEvent();
}

class Clipboard {
  const Clipboard._();

  static const Clipboard instance = Clipboard._();

  bool isSupported() => unsupported();

  bool isChangeMonitoringSupported() => unsupported();

  void read(void Function(bool, ClipboardData) callback) => unsupported();

  void readText(void Function(bool, String?) callback) => unsupported();

  void readHtml(void Function(bool, String?) callback) => unsupported();

  void readImage(void Function(bool, Image?) callback) => unsupported();

  void readFilePaths(void Function(bool, List<String>) callback) =>
      unsupported();

  bool write(ClipboardData data) => unsupported();

  bool writeText(String text) => unsupported();

  bool writeHtml(String html) => unsupported();

  bool writeImage(Image? image) => unsupported();

  bool writeFilePaths(List<String> filePaths) => unsupported();

  bool clear() => unsupported();

  bool isMonitoring() => unsupported();

  Future<ClipboardData> readAsync() {
    final completer = Completer<ClipboardData>();
    read((success, value) {
      if (success) {
        completer.complete(value);
      } else {
        completer.completeError(StateError('Clipboard operation failed'));
      }
    });
    return completer.future;
  }

  Future<String?> readTextAsync() {
    final completer = Completer<String?>();
    readText((success, value) {
      if (success) {
        completer.complete(value);
      } else {
        completer.completeError(StateError('Clipboard operation failed'));
      }
    });
    return completer.future;
  }

  Future<String?> readHtmlAsync() {
    final completer = Completer<String?>();
    readHtml((success, value) {
      if (success) {
        completer.complete(value);
      } else {
        completer.completeError(StateError('Clipboard operation failed'));
      }
    });
    return completer.future;
  }

  Future<Image?> readImageAsync() {
    final completer = Completer<Image?>();
    readImage((success, value) {
      if (success) {
        completer.complete(value);
      } else {
        completer.completeError(StateError('Clipboard operation failed'));
      }
    });
    return completer.future;
  }

  Future<List<String>> readFilePathsAsync() {
    final completer = Completer<List<String>>();
    readFilePaths((success, value) {
      if (success) {
        completer.complete(value);
      } else {
        completer.completeError(StateError('Clipboard operation failed'));
      }
    });
    return completer.future;
  }

  ListenerId addListener(FutureOr<void> Function(ClipboardEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
