// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'support.dart';
import 'unsupported.dart';

/// One `ProtocolHandlerEvent`, in its concrete form.
sealed class ProtocolHandlerEvent {
  const ProtocolHandlerEvent();
}

final class ProtocolUrlReceivedEvent extends ProtocolHandlerEvent {
  const ProtocolUrlReceivedEvent({required this.url});

  final String? url;
}

class ProtocolHandler {
  const ProtocolHandler._();

  static const ProtocolHandler instance = ProtocolHandler._();

  bool isSupported() => unsupported();

  bool register(String scheme) => unsupported();

  String? getInitialUrl() => unsupported();

  ListenerId addListener(
    FutureOr<void> Function(ProtocolHandlerEvent) callback,
  ) => unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
