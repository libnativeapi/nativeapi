// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';
import 'dart:ffi' as ffi;

import 'package:cnativeapi/cnativeapi.dart' as c;
import 'package:ffi/ffi.dart' as pkg_ffi;

import 'image.dart';

import 'support.dart';

import 'callbacks.dart';

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

  factory ClipboardData.fromNative(c.native_clipboard_data_t raw) =>
      ClipboardData(
        text: raw.text == ffi.nullptr
            ? null
            : raw.text.cast<pkg_ffi.Utf8>().toDartString(),
        html: raw.html == ffi.nullptr
            ? null
            : raw.html.cast<pkg_ffi.Utf8>().toDartString(),
        image: raw.image == 0
            ? null
            : Image.fromHandle(c.native_handle_retain(raw.image)),
        filePaths: [
          for (var i = 0; i < raw.file_paths.count; i++)
            if (raw.file_paths.items[i] != ffi.nullptr)
              raw.file_paths.items[i].cast<pkg_ffi.Utf8>().toDartString(),
        ],
      );

  /// Allocates the C form; free it with [freeNative].
  ffi.Pointer<c.native_clipboard_data_t> allocNative() {
    final pointer = pkg_ffi.calloc<c.native_clipboard_data_t>();
    pointer.ref.text = text?.toNativeUtf8().cast<ffi.Char>() ?? ffi.nullptr;
    pointer.ref.html = html?.toNativeUtf8().cast<ffi.Char>() ?? ffi.nullptr;
    pointer.ref.image = image?.nativeHandle ?? 0;
    final filePathsItems = pkg_ffi.calloc<ffi.Pointer<ffi.Char>>(
      filePaths.length,
    );
    for (var i = 0; i < filePaths.length; ++i) {
      filePathsItems[i] = filePaths[i].toNativeUtf8().cast<ffi.Char>();
    }
    pointer.ref.file_paths.items = filePathsItems;
    pointer.ref.file_paths.count = filePaths.length;
    return pointer;
  }

  static void freeNative(ffi.Pointer<c.native_clipboard_data_t> pointer) {
    if (pointer.ref.text != ffi.nullptr) {
      pkg_ffi.calloc.free(pointer.ref.text);
    }
    if (pointer.ref.html != ffi.nullptr) {
      pkg_ffi.calloc.free(pointer.ref.html);
    }
    for (var i = 0; i < pointer.ref.file_paths.count; ++i) {
      pkg_ffi.calloc.free(pointer.ref.file_paths.items[i]);
    }
    pkg_ffi.calloc.free(pointer.ref.file_paths.items);
    pkg_ffi.calloc.free(pointer);
  }
}

/// One `ClipboardEvent`, in its concrete form.
sealed class ClipboardEvent {
  const ClipboardEvent();

  /// Reads the event out of its C form. Returns null for a variant this
  /// binding does not know about.
  static ClipboardEvent? fromNative(c.native_clipboard_event_t raw) {
    if (raw.typeAsInt ==
        c
            .native_clipboard_event_type_t
            .NATIVE_CLIPBOARD_EVENT_TYPE_CHANGED
            .value) {
      return ClipboardChangedEvent();
    }
    return null;
  }
}

final class ClipboardChangedEvent extends ClipboardEvent {
  const ClipboardChangedEvent();
}

class Clipboard {
  const Clipboard._();

  /// The shared instance backed by the native singleton.
  static const Clipboard instance = Clipboard._();

  bool isSupported() {
    return c.native_clipboard_is_supported();
  }

  bool isChangeMonitoringSupported() {
    return c.native_clipboard_is_change_monitoring_supported();
  }

  void read(void Function(bool, ClipboardData) callback) {
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Bool,
            ffi.Pointer<c.native_clipboard_data_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          bool arg0,
          ffi.Pointer<c.native_clipboard_data_t> arg1,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          try {
            if (c.native_event_delivery_is_active(delivery)) {
              callback(arg0, ClipboardData.fromNative(arg1.ref));
            }
          } finally {
            c.native_event_delivery_complete(delivery, true);
          }
        });
    c.native_clipboard_read(
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
  }

  void readText(void Function(bool, String?) callback) {
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Bool,
            ffi.Pointer<ffi.Char>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          bool arg0,
          ffi.Pointer<ffi.Char> arg1,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          try {
            if (c.native_event_delivery_is_active(delivery)) {
              callback(
                arg0,
                arg1 == ffi.nullptr
                    ? null
                    : arg1.cast<pkg_ffi.Utf8>().toDartString(),
              );
            }
          } finally {
            c.native_event_delivery_complete(delivery, true);
          }
        });
    c.native_clipboard_read_text(
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
  }

  void readHtml(void Function(bool, String?) callback) {
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Bool,
            ffi.Pointer<ffi.Char>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          bool arg0,
          ffi.Pointer<ffi.Char> arg1,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          try {
            if (c.native_event_delivery_is_active(delivery)) {
              callback(
                arg0,
                arg1 == ffi.nullptr
                    ? null
                    : arg1.cast<pkg_ffi.Utf8>().toDartString(),
              );
            }
          } finally {
            c.native_event_delivery_complete(delivery, true);
          }
        });
    c.native_clipboard_read_html(
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
  }

  void readImage(void Function(bool, Image?) callback) {
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
    c.native_clipboard_read_image(
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
  }

  void readFilePaths(void Function(bool, List<String>) callback) {
    final callbackCallable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Bool,
            ffi.Pointer<c.native_string_list_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          bool arg0,
          ffi.Pointer<c.native_string_list_t> arg1,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          try {
            if (c.native_event_delivery_is_active(delivery)) {
              callback(arg0, [
                for (var i = 0; i < arg1.ref.count; i++)
                  if (arg1.ref.items[i] != ffi.nullptr)
                    arg1.ref.items[i].cast<pkg_ffi.Utf8>().toDartString(),
              ]);
            }
          } finally {
            c.native_event_delivery_complete(delivery, true);
          }
        });
    c.native_clipboard_read_file_paths(
      callbackCallable.nativeFunction,
      NativeCallbacks.userData(callbackCallable),
      NativeCallbacks.release,
    );
  }

  bool write(ClipboardData data) {
    if (!_validText(data.text) ||
        !_validText(data.html) ||
        data.filePaths.any((path) => !_validText(path))) {
      return false;
    }
    final dataPointer = data.allocNative();
    final result = c.native_clipboard_write(dataPointer.ref);
    ClipboardData.freeNative(dataPointer);
    return result;
  }

  bool writeText(String text) {
    if (!_validText(text)) {
      return false;
    }
    final textNative = text.toNativeUtf8().cast<ffi.Char>();
    final result = c.native_clipboard_write_text(textNative);
    pkg_ffi.calloc.free(textNative);
    return result;
  }

  bool writeHtml(String html) {
    if (!_validText(html)) {
      return false;
    }
    final htmlNative = html.toNativeUtf8().cast<ffi.Char>();
    final result = c.native_clipboard_write_html(htmlNative);
    pkg_ffi.calloc.free(htmlNative);
    return result;
  }

  bool writeImage(Image? image) {
    return c.native_clipboard_write_image(image?.nativeHandle ?? 0);
  }

  bool writeFilePaths(List<String> filePaths) {
    if (filePaths.any((path) => !_validText(path))) {
      return false;
    }
    final filePathsItems = pkg_ffi.calloc<ffi.Pointer<ffi.Char>>(
      filePaths.length,
    );
    for (var i = 0; i < filePaths.length; i++) {
      filePathsItems[i] = filePaths[i].toNativeUtf8().cast<ffi.Char>();
    }
    final filePathsList = pkg_ffi.calloc<c.native_string_list_t>();
    filePathsList.ref.items = filePathsItems;
    filePathsList.ref.count = filePaths.length;
    final result = c.native_clipboard_write_file_paths(filePathsList.ref);
    for (var i = 0; i < filePaths.length; i++) {
      pkg_ffi.calloc.free(filePathsItems[i]);
    }
    pkg_ffi.calloc.free(filePathsItems);
    pkg_ffi.calloc.free(filePathsList);
    return result;
  }

  bool clear() {
    return c.native_clipboard_clear();
  }

  bool isMonitoring() {
    return c.native_clipboard_is_monitoring();
  }

  static bool _validText(String? value) {
    if (value == null) return true;
    final units = value.codeUnits;
    for (var i = 0; i < units.length; ++i) {
      final unit = units[i];
      if (unit == 0) return false;
      if (unit >= 0xd800 && unit <= 0xdbff) {
        if (++i == units.length || units[i] < 0xdc00 || units[i] > 0xdfff) {
          return false;
        }
      } else if (unit >= 0xdc00 && unit <= 0xdfff) {
        return false;
      }
    }
    return true;
  }

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

  /// Registers [callback] for every `ClipboardEvent` this `Clipboard` emits.
  ///
  /// Delivered on the registering isolate, including events from native UI threads.
  /// The callback may return a Future; borrowed handles stay valid until it completes.
  /// Events queued before removal are skipped if their callback has not started.
  ListenerId addListener(FutureOr<void> Function(ClipboardEvent) callback) {
    final callable =
        ffi.NativeCallable<
          ffi.Void Function(
            ffi.Pointer<c.native_clipboard_event_t>,
            ffi.Uint64,
            ffi.Pointer<ffi.Void>,
          )
        >.listener((
          ffi.Pointer<c.native_clipboard_event_t> event,
          int delivery,
          ffi.Pointer<ffi.Void> _,
        ) {
          unawaited(
            NativeCallbacks.deliverEvent(
              delivery,
              () => event == ffi.nullptr
                  ? null
                  : ClipboardEvent.fromNative(event.ref),
              callback,
            ),
          );
        });
    return c.native_clipboard_add_listener_async(
      callable.nativeFunction,
      NativeCallbacks.userData(callable),
      NativeCallbacks.release,
    );
  }

  /// Unregisters a listener. Returns false if unknown.
  bool removeListener(ListenerId listenerId) =>
      c.native_clipboard_remove_listener(listenerId);
}
