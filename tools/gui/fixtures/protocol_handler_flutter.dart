// Debug-only endpoint for the real OS deep-link verification runner.
// Build through verify_protocol_handler.py; never ship this entry point.
import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:material_ui/material_ui.dart';
import 'package:protocol_handler/protocol_handler.dart';

class RecordingListener with ProtocolListener {
  final urls = <String>[];

  @override
  void onProtocolUrlReceived(String url) => urls.add(url);
}

Future<void> main() async {
  var debug = false;
  assert(debug = true);
  if (!debug) throw StateError('Verification requires a debug build');
  WidgetsFlutterBinding.ensureInitialized();
  final registered = await protocolHandler.register('myprotocol');
  final invalid = await protocolHandler.register('1 invalid:');
  final initialAtRead = await protocolHandler.getInitialUrl();
  final listener = RecordingListener();
  final streamUrls = <String?>[];
  StreamSubscription<String?>? subscription;
  void listen() {
    if (!protocolHandler.listeners.contains(listener)) {
      protocolHandler.addListener(listener);
    }
    subscription ??= protocolHandler.onUrlReceived.listen(streamUrls.add);
  }

  listen();
  final server = await HttpServer.bind(InternetAddress.loopbackIPv4, 49227);
  runApp(
    const MaterialApp(
      home: Scaffold(
        body: Center(
          child: Text('Protocol handler verification — no input required'),
        ),
      ),
    ),
  );
  await for (final request in server) {
    switch (request.uri.path) {
      case '/remove-listener':
        protocolHandler.removeListener(listener);
      case '/stop':
        protocolHandler.removeListener(listener);
        await subscription?.cancel();
        subscription = null;
        // Allow cancellation of the facade's internal subscription to settle.
        await Future<void>.delayed(Duration.zero);
      case '/listen':
        listen();
      case '/snapshot':
        protocolHandler.listeners.clear();
      case '/state':
        break;
      default:
        request.response.statusCode = HttpStatus.notFound;
    }
    request.response.headers.contentType = ContentType.json;
    request.response.write(
      jsonEncode({
        'pid': pid,
        'registered': registered,
        'invalid': invalid,
        'initialAtRead': initialAtRead,
        'initial': await protocolHandler.getInitialUrl(),
        'listener': listener.urls,
        'stream': streamUrls,
        'hasListeners': protocolHandler.hasListeners,
        'listeners': protocolHandler.listeners.length,
      }),
    );
    await request.response.close();
  }
}
