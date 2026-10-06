// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import '../unsupported.dart';

class EventDecision {
  EventDecision.fromHandle(this.nativeHandle);
  EventDecision.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  bool accept() => unsupported();

  bool cancel() => unsupported();

  bool get isPending => unsupported();
}

class EventRequest {
  EventRequest.fromHandle(this.nativeHandle);
  EventRequest.borrowed(this.nativeHandle);

  final int nativeHandle;

  void dispose() => unsupported();

  bool get isCancelable => unsupported();

  bool get isCancelled => unsupported();

  bool get isPending => unsupported();

  bool cancel() => unsupported();

  EventDecision? defer() => unsupported();
}
