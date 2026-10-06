// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'support.dart';
import 'unsupported.dart';

/// One `NotificationEvent`, in its concrete form.
sealed class NotificationEvent {
  const NotificationEvent();
}

final class NotificationActivatedEvent extends NotificationEvent {
  const NotificationActivatedEvent({required this.argument});

  final String? argument;
}

class NotificationManager {
  const NotificationManager._();

  static const NotificationManager instance = NotificationManager._();

  bool isSupported() => unsupported();

  bool initialize() => unsupported();

  void shutdown() => unsupported();

  bool show(String title, String message, String tag, String buttonLabel) =>
      unsupported();

  bool remove(String tag) => unsupported();

  String? getLastError() => unsupported();

  ListenerId addListener(FutureOr<void> Function(NotificationEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
