// Headless native-host regression: no widgets, windows or pointer input.
import 'dart:ui';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

late final AppLifecycleListener exitListener;

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  const channel = MethodChannel('nativeapi/test-exit');
  exitListener = AppLifecycleListener(
    onExitRequested: () async {
      final allow = await channel.invokeMethod<bool>('request');
      return allow == true ? AppExitResponse.exit : AppExitResponse.cancel;
    },
  );
  await SystemChannels.platform.invokeMethod<void>(
    'System.initializationComplete',
  );
  await channel.invokeMethod<void>('ready');
}
