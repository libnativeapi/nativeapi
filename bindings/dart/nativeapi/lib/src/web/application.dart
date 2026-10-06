// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'dart:async';

import 'foundation/event_request.dart';
import 'menu.dart';
import 'window.dart';
import 'support.dart';
import 'unsupported.dart';

enum Brightness {
  system(0),
  light(1),
  dark(2);

  const Brightness(this.value);
  final int value;

  static Brightness fromValue(int value) => switch (value) {
    0 => Brightness.system,
    1 => Brightness.light,
    2 => Brightness.dark,
    _ => Brightness.system,
  };
}

/// One `ApplicationEvent`, in its concrete form.
sealed class ApplicationEvent {
  const ApplicationEvent();
}

final class ApplicationStartedEvent extends ApplicationEvent {
  const ApplicationStartedEvent();
}

final class ApplicationExitingEvent extends ApplicationEvent {
  const ApplicationExitingEvent({required this.exitCode});

  final int exitCode;
}

final class ApplicationActivatedEvent extends ApplicationEvent {
  const ApplicationActivatedEvent();
}

final class ApplicationDeactivatedEvent extends ApplicationEvent {
  const ApplicationDeactivatedEvent();
}

final class ApplicationQuitRequestedEvent extends ApplicationEvent {
  const ApplicationQuitRequestedEvent({required this.request});

  final EventRequest request;
}

class Application {
  const Application._();

  static const Application instance = Application._();

  int run() => unsupported();

  int runWithWindow(Window? window) => unsupported();

  void quit(int exitCode) => unsupported();

  bool isRunning() => unsupported();

  bool isSingleInstance() => unsupported();

  bool show() => unsupported();

  bool hide() => unsupported();

  bool isVisible() => unsupported();

  bool setIcon(String iconPath) => unsupported();

  bool setDockIconVisible(bool visible) => unsupported();

  bool setProgressBar(double progress) => unsupported();

  bool setBadgeLabel(String label) => unsupported();

  bool setBrightness(Brightness brightness) => unsupported();

  bool setMenuBar(Menu? menu) => unsupported();

  Window? getPrimaryWindow() => unsupported();

  void setPrimaryWindow(Window? window) => unsupported();

  List<Window> getAllWindows() => unsupported();

  ListenerId addListener(FutureOr<void> Function(ApplicationEvent) callback) =>
      unsupported();

  bool removeListener(ListenerId listenerId) => unsupported();
}
