import 'dart:async';
import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:flutter/services.dart';

import 'native_sign.dart';
import 'sign_controller.dart';

/// An independent native sign hosted by Flutter's macOS PlatformView.
class EmbeddedSignPreview extends StatefulWidget {
  const EmbeddedSignPreview({super.key, required this.controller});

  final SignController controller;

  @override
  State<EmbeddedSignPreview> createState() => _EmbeddedSignPreviewState();
}

class _EmbeddedSignPreviewState extends State<EmbeddedSignPreview>
    with AutomaticKeepAliveClientMixin {
  static const _channel = MethodChannel('dev.nativeapi.tray_sign/typography');
  NativeSign? _sign;
  Future<void> _registration = Future.value();
  Object? _configuration;
  int? _platformViewId;
  bool _registered = false, _attached = false, _disposed = false;
  String? _error;

  bool get isAttached => _attached;

  @override
  bool get wantKeepAlive => true;

  @override
  void initState() {
    super.initState();
    if (!Platform.isMacOS) return;
    try {
      _sign = NativeSign(scale: 40 / 28, onLayout: _syncHost);
      widget.controller.addListener(_update);
      _update();
      _registration = _register();
    } catch (error) {
      _error = '$error';
    }
  }

  Future<void> _register() async {
    try {
      // Retain the NSView before asynchronous PlatformView creation. The
      // factory receives a registration key, never a potentially stale pointer.
      await _channel.invokeMethod<void>('registerEmbeddedSign', {
        'root': _sign!.view.nativeObject.address,
      });
      if (!_disposed) setState(() => _registered = true);
    } catch (error) {
      if (!_disposed) setState(() => _error = '$error');
    }
  }

  @override
  void didUpdateWidget(EmbeddedSignPreview oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.controller != widget.controller && Platform.isMacOS) {
      oldWidget.controller.removeListener(_update);
      widget.controller.addListener(_update);
      _configuration = null;
      _update();
    }
  }

  void _update() {
    if (_disposed || _sign == null) return;
    final controller = widget.controller;
    final configuration = (
      controller.style,
      controller.content,
      controller.green,
      controller.english,
      controller.right,
      32.0,
    );
    if (_configuration == configuration) return;
    _configuration = configuration;
    _sign!.view.isVisible = true;
    _sign!.update(
      style: controller.style,
      content: controller.content,
      green: controller.green,
      english: controller.english,
      right: controller.right,
      barHeight: 32,
    );
  }

  Future<void> _syncHost() async {
    final id = _platformViewId, sign = _sign;
    if (_disposed || id == null || sign == null) return;
    try {
      final attached = await _channel.invokeMethod<bool>('layoutEmbeddedSign', {
        'id': id,
        'width': sign.width,
        'height': sign.height,
      });
      if (!_disposed) _attached = attached ?? false;
    } catch (error) {
      if (!_disposed) setState(() => _error = '$error');
    }
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    if (!Platform.isMacOS) {
      return const Text('Native sign previews are available on macOS.');
    }
    return SizedBox(
      height: 80,
      child: _error != null
          ? Center(child: Text('Preview unavailable: $_error'))
          : !_registered
          ? const SizedBox.shrink()
          : AppKitView(
              viewType: 'dev.nativeapi.tray_sign/preview',
              layoutDirection: TextDirection.ltr,
              creationParams: {'sign': _sign!.view.nativeObject.address},
              creationParamsCodec: const StandardMessageCodec(),
              onPlatformViewCreated: (id) {
                _platformViewId = id;
                unawaited(_syncHost());
              },
            ),
    );
  }

  Future<void> _release(NativeSign sign) async {
    await _registration;
    try {
      // Detach all native hosts before releasing the nativeapi handles.
      await _channel.invokeMethod<void>('unregisterEmbeddedSign', {
        'root': sign.view.nativeObject.address,
      });
    } catch (error) {
      debugPrint('Embedded sign release: $error');
    } finally {
      sign.dispose();
    }
  }

  @override
  void dispose() {
    _disposed = true;
    widget.controller.removeListener(_update);
    if (_sign case final sign?) unawaited(_release(sign));
    super.dispose();
  }
}
