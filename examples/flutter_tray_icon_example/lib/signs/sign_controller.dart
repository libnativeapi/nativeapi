import 'dart:async';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:nativeapi/nativeapi.dart' as na;

import 'native_sign.dart';
import 'sign_style.dart';

final class SignController extends ChangeNotifier {
  SignController(
    this.settingsWindow, {
    required this.id,
    required this.onSelect,
    required na.TrayIcon tray,
    required SignStyle initialStyle,
  }) {
    style = initialStyle;
    try {
      if (Platform.isMacOS && na.TrayManager.instance.isSupported()) {
        _tray = tray;
        _traySign = NativeSign(onLayout: _onSignLayout);
        _trayRoot = na.View.create();
        if (_trayRoot == null) {
          throw StateError('Could not create tray content');
        }
        _trayRoot!
          ..backgroundColor = const na.Color(r: 0, g: 0, b: 0, a: 0)
          ..addSubview(_traySign!.view);
        _tray!.contentView = _trayRoot;
      }
      _windowListener = na.WindowManager.instance.addListener((event) {
        if (event is na.WindowClosedEvent &&
            event.windowId == _previewWindow?.id) {
          _releasePreview();
        }
      });
      _update();
      // AppKit initially reports zero or a provisional height. Synchronize
      // after placement and when a display's menu-bar height changes.
      if (_tray != null) {
        _trayLayoutTimer = Timer.periodic(const Duration(milliseconds: 500), (
          _,
        ) {
          final height = _tray!.getBounds().height;
          if (height > 0 && height != _trayHeight) _update();
        });
      }
    } catch (e) {
      error = '$e';
      _releaseTray();
    }
  }

  final na.Window? settingsWindow;
  final int id;
  final VoidCallback onSelect;
  SignStyle style = SignStyle.missing;
  final Map<SignStyle, SignContent> _contents = {
    for (final style in SignStyle.values) style: style.defaults,
  };
  SignContent get content => _contents[style]!;
  List<String> get fields => style.fields;
  SignContent get defaults => style.defaults;
  String get city => content.primary;
  String get romanized => content.secondary;
  String get displayTitle => content.heading(style);
  bool green = false;
  bool english = true;
  bool right = true;
  double _trayHeight = 32;
  String? error;
  static const _traySidePadding = 4.0;
  NativeSign? _traySign;
  na.View? _trayRoot;
  NativeSign? _previewSign;
  na.TrayIcon? _tray;
  na.ListenerId? _windowListener;
  bool _disposed = false;
  Timer? _trayLayoutTimer;
  na.Window? _previewWindow;
  na.View? _previewRoot;
  Future<void> _previewConfiguration = Future.value();
  bool _openingPreview = false;

  bool get trayAvailable => _tray != null;
  double get trayWidth => _trayRoot?.preferredSize.width ?? 0;
  double get trayHeight => _trayHeight;

  /// Invalid input leaves the current sign intact.
  String? setPlace(String name, String latin) {
    return setContent(name, latin.toUpperCase(), content.value, content.detail);
  }

  String? setContent(
    String primary,
    String secondary,
    String value,
    String detail,
  ) {
    final next = SignContent(
      primary.trim(),
      secondary.trim(),
      value.trim(),
      detail.trim(),
    );
    final problem = next.validate(style);
    if (problem != null) return problem;
    _contents[style] = next;
    _update();
    return null;
  }

  void selectStyle(int index) {
    if (index < 0 || index >= SignStyle.values.length) return;
    style = SignStyle.values[index];
    english = true;
    right = true;
    _update();
  }

  void restoreDefaults() {
    _contents[style] = style.defaults;
    english = true;
    right = true;
    if (style == SignStyle.missing) green = false;
    _update();
  }

  void setGreen(bool value) {
    green = value;
    _update();
  }

  void handleTrayClick() => showSettings();

  void setEnglish(bool value) {
    english = style == SignStyle.missing || value;
    _update();
  }

  void setRight(bool value) {
    right = value;
    _update();
  }

  void _update() {
    final availableHeight = _tray?.getBounds().height ?? 0;
    if (availableHeight > 0) _trayHeight = availableHeight;
    for (final sign in [_traySign, _previewSign]) {
      sign?.update(
        style: style,
        content: content,
        green: green,
        english: style == SignStyle.missing || english,
        right: right,
        barHeight: _trayHeight,
      );
    }
    const action = 'Click to edit, right-click for menu';
    _tray?.setTooltip('${style.label} · $displayTitle · $action');
    _layoutPreview();
    notifyListeners();
  }

  void _onSignLayout() {
    if (_disposed) return;
    _layoutTray();
    _layoutPreview();
    notifyListeners();
  }

  void _layoutTray() {
    final sign = _traySign, root = _trayRoot;
    if (sign == null || root == null) return;
    // Keep spacing outside the painted sign, using a transparent tray host.
    sign.view.frame = na.Rectangle(
      x: _traySidePadding,
      y: 0,
      width: sign.width,
      height: sign.height,
    );
    root.preferredSize = na.Size(
      width: sign.width + _traySidePadding * 2,
      height: sign.height,
    );
  }

  void showSettings() {
    onSelect();
    settingsWindow?.show();
    settingsWindow?.focus();
  }

  void hideSettings() => settingsWindow?.hide();

  Future<void> showPreview() async {
    if (_disposed || _openingPreview) return;
    _openingPreview = true;
    try {
      if (_previewWindow == null) {
        _previewWindow = na.Window.create();
        final window = _previewWindow;
        if (window == null) throw StateError('Could not create preview window');
        _previewConfiguration = _configurePreview(window);
        _previewRoot = _previewWindow?.contentView;
        if (_previewRoot == null) {
          throw StateError('Could not create preview window');
        }
        _previewSign = NativeSign(scale: 4, onLayout: _onSignLayout);
        _previewRoot!
          ..backgroundColor = const na.Color(r: 238, g: 241, b: 237, a: 255)
          ..addSubview(_previewSign!.view);
        _previewWindow!
          ..title = 'Enlarged sign preview'
          ..isResizable = false;
        await _previewConfiguration;
        if (_disposed || _previewWindow != window) return;
        _update();
        window.center();
      }
      _previewWindow!.show();
      _previewWindow!.focus();
    } catch (e) {
      _releasePreview(close: true);
      if (!_disposed) {
        error = '$e';
        notifyListeners();
      }
    } finally {
      _openingPreview = false;
    }
  }

  Future<void> _configurePreview(na.Window window) async {
    if (!Platform.isMacOS) return;
    await const MethodChannel('dev.nativeapi.tray_sign/typography')
        .invokeMethod<void>('configurePreviewWindow', {
          'address': window.nativeObject.address,
        });
  }

  void _layoutPreview() {
    final sign = _previewSign;
    if (sign == null) return;
    _previewWindow!.contentSize = na.Size(
      width: sign.width + 48,
      height: sign.height + 48,
    );
    sign.view.frame = na.Rectangle(
      x: 24,
      y: 24,
      width: sign.width,
      height: sign.height,
    );
  }

  void _releasePreview({bool close = false}) {
    final root = _previewRoot, sign = _previewSign, window = _previewWindow;
    _previewRoot = null;
    _previewSign = null;
    _previewWindow = null;
    if (window == null) return;
    void release() {
      if (close) window.close();
      root?.clearSubviews();
      sign?.dispose();
      root?.dispose();
      window.dispose();
    }

    // Keep the native window alive while AppKit ownership is configured.
    // Also let a WindowClosed notification return before disposing its sender.
    unawaited(
      _previewConfiguration.then(
        (_) => release(),
        onError: (Object _) => release(),
      ),
    );
  }

  void _releaseTray() {
    _tray?.contentView = null;
    _trayRoot?.clearSubviews();
    _traySign?.dispose();
    _trayRoot?.dispose();
    _tray = null;
    _traySign = null;
    _trayRoot = null;
  }

  @override
  void dispose() {
    if (_disposed) return;
    _disposed = true;
    _trayLayoutTimer?.cancel();
    if (_windowListener case final listener?) {
      na.WindowManager.instance.removeListener(listener);
      _windowListener = null;
    }
    _releasePreview(close: true);
    _releaseTray();
    super.dispose();
  }
}
