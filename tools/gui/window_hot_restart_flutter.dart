// Target for flutter_hot_restart_test.py. Run in flutter_window_example so its
// unmodified Windows runner exercises the same lifecycle as a generated app.
import 'dart:async';
import 'dart:convert';
import 'dart:developer';
import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:nativeapi/nativeapi.dart' as native;

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  final window = native.WindowManager.instance.getCurrent();
  final generation = DateTime.now().microsecondsSinceEpoch;
  var closeCount = 0;
  var allowClose = false;
  registerExtension('ext.nativeapi.hotRestartState', (_, _) async {
    return ServiceExtensionResponse.result(
      jsonEncode({
        'pid': pid,
        'id': window?.id,
        'hwnd': window?.nativeObject.address,
        'generation': generation,
        'closeCount': closeCount,
      }),
    );
  });
  stdout.writeln(
    'HOT_RESTART_REPORT ${jsonEncode({'pid': pid, 'id': window?.id, 'hwnd': window?.nativeObject.address})}',
  );
  if (window != null) {
    window
      ..title = 'NativeAPI hot restart regression'
      ..titleBarStyle = native.TitleBarStyle.hidden
      ..minimumSize = const native.Size(width: 800, height: 600)
      ..show()
      ..focus();
    window.addListener((event) {
      if (event is native.WindowCloseRequestedEvent) {
        closeCount++;
        if (!allowClose) event.request.cancel();
        stdout.writeln('HOT_RESTART_CLOSE ${window.id}');
      }
    });
  }
  registerExtension('ext.nativeapi.closeForTest', (_, parameters) async {
    allowClose = parameters['allow'] == 'true';
    // Let the RPC return before the Flutter engine shuts down.
    Timer(const Duration(milliseconds: 500), () => window?.close());
    return ServiceExtensionResponse.result(
      jsonEncode({'requested': window != null}),
    );
  });
  runApp(
    Directionality(
      textDirection: TextDirection.ltr,
      child: ColoredBox(
        color: const Color(0xFF244865),
        child: Center(
          child: Text(
            window == null
                ? 'FAIL: current window is null'
                : 'Hot restart ready',
          ),
        ),
      ),
    ),
  );
}
