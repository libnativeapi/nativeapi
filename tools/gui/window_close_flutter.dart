// ignore_for_file: invalid_use_of_internal_member, implementation_imports
// Actual Flutter window/controller and nativeapi FFI; windows stay unshown.
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' as fw;
import 'package:flutter/src/widgets/_window_macos.dart' as fm;
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/windowing.dart' as bridge;

const channel = MethodChannel('nativeapi/test-window-close');

class Host with fw.RegularWindowControllerDelegate {
  int requested = 0, destroyed = 0;
  bool allow = false;
  @override
  void onWindowCloseRequested(fw.RegularWindowController controller) {
    requested++;
    if (allow) controller.destroy();
  }

  @override
  void onWindowDestroyed() {
    destroyed++;
  }
}

Future<void> settle() => Future<void>.delayed(const Duration(milliseconds: 40));
Future<void> check(bool ok, String label) =>
    channel.invokeMethod<void>('check', {'ok': ok, 'label': label});
Future<bool?> perform(
  fw.RegularWindowController controller, {
  bool force = false,
}) => channel.invokeMethod<bool>('perform', {
  'handle': (controller as fm.WindowControllerMacOS).windowHandle.address,
  'force': force,
});

Future<void> tests() async {
  final host = Host();
  final controller = fw.RegularWindowController(
    size: const Size(160, 100),
    delegate: host,
  );
  final native = bridge.nativeWindowOf(controller)!;
  final alias = bridge.nativeWindowOf(controller)!;
  await check(
    native.id == alias.id,
    'Flutter controller aliases share native identity',
  );
  await perform(controller);
  await settle();
  await check(
    host.requested == 1 && host.destroyed == 0,
    'no native listener preserves the actual Flutter host refusal',
  );
  await check(
    native.close(),
    'Dart Window.close starts the real host close path',
  );
  await settle();
  await check(
    host.requested == 2 && host.destroyed == 0,
    'public close also preserves Flutter host refusal without listeners',
  );

  var events = 0;
  var mode = 'veto';
  na.EventRequest? request;
  na.EventDecision? first, second;
  final listener = native.addListener((event) {
    if (event is! na.WindowCloseRequestedEvent) return;
    events++;
    request = event.request;
    if (mode == 'veto') event.request.cancel();
    if (mode == 'defer') first = event.request.defer();
  });
  final aliasListener = alias.addListener((event) {
    if (event is na.WindowCloseRequestedEvent && mode == 'defer') {
      second = event.request.defer();
    }
  });
  await perform(controller);
  await settle();
  await check(
    events == 1 && host.requested == 2 && host.destroyed == 0,
    'native close veto precedes Flutter controller callback',
  );
  native.close();
  await settle();
  await check(
    events == 2 && host.requested == 2,
    'Dart public close uses the same cancellable gate',
  );
  mode = 'defer';
  native.close();
  await settle();
  await check(
    events == 3 && !request!.isPending && first!.isPending && second!.isPending,
    'Dart borrowed request expires while explicit votes survive delivery',
  );
  native.close();
  await perform(controller);
  await settle();
  await check(
    events == 3 && host.requested == 2,
    'public and native attempts coalesce during Dart confirmation',
  );
  first!.accept();
  await settle();
  await check(
    host.requested == 2,
    'one Dart alias cannot approve the other vote',
  );
  second!.accept();
  await settle();
  await check(
    host.requested == 3 && host.destroyed == 0,
    'Dart approval resumes the real Flutter host once and preserves its refusal',
  );
  first!.dispose();
  second!.dispose();
  mode = 'allow';
  host.allow = true;
  native.close();
  await settle();
  await check(
    events == 4 && host.requested == 4 && host.destroyed == 1,
    'approved retry lets Flutter destroy its own controller exactly once',
  );
  await check(
    bridge.nativeWindowOf(controller) == null,
    'nativeWindowOf returns null for the actually destroyed Flutter controller',
  );
  await check(
    !native.close(),
    'an old native wrapper cannot close a destroyed Flutter window',
  );
  native.removeListener(listener);
  alias.removeListener(aliasListener);
  native.dispose();
  alias.dispose();

  final forcedHost = Host();
  final forcedController = fw.RegularWindowController(
    size: const Size(160, 100),
    delegate: forcedHost,
  );
  final forced = bridge.nativeWindowOf(forcedController)!;
  na.EventDecision? old;
  var required = 0;
  final forcedListener = forced.addListener((event) {
    if (event is! na.WindowCloseRequestedEvent) return;
    if (event.request.isCancelable) {
      old = event.request.defer();
    } else {
      required++;
      if (event.request.cancel() || event.request.defer() != null)
        throw StateError('forced close was cancellable');
    }
  });
  forced.close();
  await settle();
  await check(
    old!.isPending && forcedHost.destroyed == 0,
    'a second real Flutter window owns a pending Dart vote',
  );
  final rawClosed = await perform(forcedController, force: true);
  await settle();
  await check(
    rawClosed == true && required == 1 && !old!.accept(),
    'raw native close emits required notification and invalidates votes while closing the OS window',
  );
  // NSWindow.close closes the OS window; Flutter's explicit destroy owns its
  // view/controller teardown. It must remain callable after a native close.
  forcedController.destroy();
  await settle();
  await check(
    forcedHost.destroyed == 1 &&
        bridge.nativeWindowOf(forcedController) == null,
    'Flutter retains and completes its own controller teardown after raw native close',
  );
  old!.dispose();
  forced.removeListener(forcedListener);
  forced.dispose();

  final ownerHost = Host();
  final ownerController = fw.RegularWindowController(
    size: const Size(160, 100),
    delegate: ownerHost,
  );
  final ownerWindow = bridge.nativeWindowOf(ownerController)!;
  na.EventDecision? ownerVote;
  var ownerRequired = 0;
  final ownerListener = ownerWindow.addListener((event) {
    if (event is! na.WindowCloseRequestedEvent) return;
    if (event.request.isCancelable)
      ownerVote = event.request.defer();
    else {
      ownerRequired++;
      if (event.request.cancel() || event.request.defer() != null)
        throw StateError('Flutter destroy was cancellable');
    }
  });
  ownerWindow.close();
  await settle();
  await check(
    ownerVote!.isPending,
    'Flutter-owned destruction begins with a pending native confirmation',
  );
  ownerController.destroy();
  await settle();
  await check(
    ownerRequired == 1 && ownerHost.destroyed == 1 && !ownerVote!.accept(),
    'actual Flutter destroy bypasses pending votes and tears down its controller once',
  );
  await check(
    bridge.nativeWindowOf(ownerController) == null && !ownerWindow.close(),
    'Flutter-owned destruction fences old wrappers and native controller handles',
  );
  ownerVote!.dispose();
  ownerWindow.removeListener(ownerListener);
  ownerWindow.dispose();
}

Future<void> main() async {
  isWindowingEnabled = true;
  WidgetsFlutterBinding.ensureInitialized();
  await channel.invokeMethod<void>('ready');
  try {
    await tests();
    await channel.invokeMethod<void>('complete');
  } catch (error, stack) {
    await channel.invokeMethod<void>('failed', '$error\n$stack');
  }
}
