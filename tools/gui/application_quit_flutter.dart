// Real macOS runner fixture for application_quit_gui_macos.py (#65).
import 'dart:async';
import 'dart:developer' as developer;
import 'dart:ui' show AppExitResponse;

import 'package:flutter/widgets.dart';
import 'package:nativeapi/nativeapi.dart' as native;

void main() => runApp(const QuitApp());

class QuitApp extends StatefulWidget {
  const QuitApp({super.key});

  @override
  State<QuitApp> createState() => _QuitAppState();
}

class _QuitAppState extends State<QuitApp> {
  bool _nativeAllows = false;
  bool _hostAllows = false;
  int _nativeRequests = 0;
  int _hostRequests = 0;
  late final int _listener;
  late final AppLifecycleListener _lifecycle;

  @override
  void initState() {
    super.initState();
    final window = native.WindowManager.instance.getCurrent()!;
    window.title = 'NativeAPI quit verification';
    window.setSize(const native.Size(width: 640, height: 420), false);
    window.center();
    _lifecycle = AppLifecycleListener(
      onExitRequested: () async {
        setState(() => _hostRequests++);
        print('HOST_QUIT $_hostRequests allow=$_hostAllows');
        await Future<void>.delayed(const Duration(milliseconds: 100));
        return _hostAllows ? AppExitResponse.exit : AppExitResponse.cancel;
      },
    );
    _listener = native.Application.instance.addListener((event) async {
      if (event is! native.ApplicationQuitRequestedEvent) return;
      setState(() => _nativeRequests++);
      print('NATIVE_QUIT $_nativeRequests allow=$_nativeAllows');
      // Exercise the asynchronous Dart acknowledgement, not just a sync veto.
      await Future<void>.delayed(const Duration(milliseconds: 100));
      if (!_nativeAllows && event.request.isCancelable) event.request.cancel();
    });
    // Keep repeated regression runs independent of the user's mouse.
    developer.registerExtension('ext.nativeapi.quitMode', (
      method,
      parameters,
    ) async {
      final mode = parameters['mode'];
      if (!const ['native', 'flutter', 'allow'].contains(mode)) {
        return developer.ServiceExtensionResponse.error(
          developer.ServiceExtensionResponse.invalidParams,
          'Expected native, flutter or allow',
        );
      }
      setState(() {
        _nativeAllows = mode != 'native';
        _hostAllows = mode == 'allow';
      });
      return developer.ServiceExtensionResponse.result('{}');
    });
  }

  @override
  void dispose() {
    native.Application.instance.removeListener(_listener);
    _lifecycle.dispose();
    super.dispose();
  }

  Widget _button(String title, VoidCallback action) => GestureDetector(
    behavior: HitTestBehavior.opaque,
    onTap: action,
    child: Padding(padding: const EdgeInsets.all(20), child: Text(title)),
  );

  @override
  Widget build(BuildContext context) => Directionality(
    textDirection: TextDirection.ltr,
    child: ColoredBox(
      color: const Color(0xffeeeeee),
      child: DefaultTextStyle(
        style: const TextStyle(color: Color(0xff111111), fontSize: 18),
        child: Column(
          children: [
            const SizedBox(height: 30),
            const Text('NativeAPI quit verification'),
            Text('Native requests: $_nativeRequests'),
            Text('Host requests: $_hostRequests'),
            _button(
              'Native cancels',
              () => setState(() {
                _nativeAllows = false;
                _hostAllows = false;
              }),
            ),
            _button(
              'Flutter cancels',
              () => setState(() {
                _nativeAllows = true;
                _hostAllows = false;
              }),
            ),
            _button(
              'Allow quit',
              () => setState(() {
                _nativeAllows = true;
                _hostAllows = true;
              }),
            ),
          ],
        ),
      ),
    ),
  );
}
