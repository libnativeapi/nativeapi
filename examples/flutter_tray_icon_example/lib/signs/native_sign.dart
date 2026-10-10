import 'dart:io';
import 'dart:math' as math;

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:nativeapi/nativeapi.dart' as na;

import 'sign_style.dart';

const signBlue = na.Color(r: 24, g: 65, b: 112, a: 255);
const signGreen = na.Color(r: 0, g: 105, b: 78, a: 255);
const _white = na.Color(r: 250, g: 250, b: 245, a: 255);
const _ink = na.Color(r: 20, g: 30, b: 37, a: 255);
const _brown = na.Color(r: 104, g: 62, b: 36, a: 255);
const _ochre = na.Color(r: 224, g: 183, b: 102, a: 255);

/// Every host owns native Views and Labels, including badges and fingerboards.
/// No Flutter painting or bitmap capture is involved.
final class NativeSign {
  NativeSign({this.scale = 1, this.onLayout}) {
    try {
      view = _keep(na.View.create());
      _post = _keep(na.View.create());
      _face = _keep(na.View.create());
      _badge = _keep(na.View.create());
      _title = _keep(na.Label.create(''));
      _caption = _keep(na.Label.create(''));
      _west = _keep(na.Label.create(''));
      _east = _keep(na.Label.create(''));
      _w = _keep(na.Label.create(''));
      _e = _keep(na.Label.create(''));
      _badgeNumber = _keep(na.Label.create(''));
      _detail = _keep(na.Label.create(''));
      view.addSubview(_post);
      view.addSubview(_face);
      for (final label in [_title, _west, _east]) {
        _face.addSubview(label);
      }
      view.addSubview(_badge);
      _badge.addSubview(_badgeNumber);
      for (final label in [_caption, _w, _e, _detail]) {
        view.addSubview(label);
      }
      for (final label in _labels) {
        label.textAlignment = na.TextAlignment.center;
      }
    } catch (_) {
      dispose();
      rethrow;
    }
  }

  final double scale;
  final void Function()? onLayout;
  final List<na.View> _owned = [];
  late final na.View view, _face, _badge, _post;
  late final na.Label _title, _caption, _west, _east, _w, _e;
  late final na.Label _badgeNumber, _detail;
  List<na.Label> get _labels => [
    _title,
    _caption,
    _west,
    _east,
    _w,
    _e,
    _badgeNumber,
    _detail,
  ];
  double width = 148;
  double _baseHeight = 32;
  bool _bilingual = true;
  bool _right = true;
  bool _disposed = false;
  SignStyle _style = SignStyle.missing;
  int _revision = 0;
  int _styledRevision = 0;
  Future<void> _styling = Future.value();
  // TrayIcon centers its content View in the menu bar. Reduce the View itself
  // so the 2 pt top/bottom margin applies to every template, outside its face.
  double get height => math.max(1, _baseHeight - 4) * scale;
  bool get isReady => _revision == _styledRevision;

  T _keep<T extends na.View>(T? value) {
    if (value == null) {
      throw StateError('Could not create the native sign view');
    }
    _owned.add(value);
    return value;
  }

  void update({
    required SignStyle style,
    required SignContent content,
    required bool green,
    required bool english,
    required bool right,
    required double barHeight,
  }) {
    _style = style;
    _revision++;
    _baseHeight = barHeight;
    _bilingual =
        style == SignStyle.missing || style == SignStyle.travel || english;
    _right = right;
    for (final label in _labels) {
      label.isVisible = false;
    }
    _post.isVisible = false;
    _badge.isVisible = false;
    view.backgroundColor = _white;
    final h = height;
    final mainSize = h * (_bilingual ? 0.35 : 0.46);
    final subSize = h * 0.185;
    _face.backgroundColor = switch (style) {
      SignStyle.guide => _brown,
      SignStyle.travel => signGreen,
      SignStyle.welcome => signBlue,
      _ => green ? signGreen : signBlue,
    };
    switch (style) {
      case SignStyle.missing:
        _text(_title, content.heading(style), mainSize, _white);
        final pinyin = content.secondary
            .split(RegExp(r'[ -]+'))
            .map(
              (word) => word.isEmpty
                  ? ''
                  : word[0].toUpperCase() + word.substring(1).toLowerCase(),
            )
            .join();
        _text(_caption, 'WoZai${pinyin}HenXiangNi', subSize, _ink);
        _text(_west, right ? '东' : '西', mainSize * 0.66, _white);
        _text(_east, right ? '西' : '东', mainSize * 0.66, _white);
        _text(_w, right ? 'E' : 'W', subSize, _ink);
        _text(_e, right ? 'W' : 'E', subSize, _ink);
        for (final label in [_caption, _w, _e]) {
          label.isVisible = _bilingual;
        }
      case SignStyle.welcome:
        _text(_title, content.heading(style), mainSize * 1.12, _white);
        _text(
          _caption,
          'WELCOME TO ${content.secondary.toUpperCase()}',
          subSize,
          _white,
        );
      case SignStyle.travel:
        _text(_title, content.primary, h * 0.28, _white);
        _text(_caption, content.secondary, h * 0.26, _ink);
        _text(_east, right ? '➜' : '←', h * 0.31, _white);
        _text(_detail, right ? '←' : '➜', h * 0.31, _ink);
        _badge.isVisible = true;
        _badge.backgroundColor = _ochre;
        _post.isVisible = true;
        _post.backgroundColor = _brown;
      case SignStyle.guide:
        _text(_title, content.primary, mainSize * 1.1, _white);
        _text(
          _caption,
          '${content.secondary.toUpperCase()}  /  ${content.value}',
          subSize,
          _white,
        );
        _badge.isVisible = true;
        _badge.backgroundColor = _white;
        _text(_badgeNumber, '▲', h * 0.34, _brown);
        _arrow(h);
    }
    if (style.hasSubtitleToggle) _caption.isVisible = _bilingual;
    _layout();
    _styleNativeControls();
  }

  void _text(na.Label label, String text, double size, na.Color color) {
    label
      ..text = text
      ..fontSize = size
      ..textColor = color
      ..isVisible = true;
  }

  void _arrow(double h) {
    _text(_right ? _east : _west, _right ? '➜' : '←', h * 0.50, _white);
  }

  void _styleNativeControls() {
    final revision = _revision;
    if (!Platform.isMacOS) {
      _styledRevision = revision;
      return;
    }
    // Keep handles alive until the main-thread AppKit styling call completes.
    _styling = _styling
        .then((_) async {
          if (_disposed || revision != _revision) return;
          await const MethodChannel('dev.nativeapi.tray_sign/typography')
              .invokeMethod<void>('style', {
                'root': view.nativeObject.address,
                'radius': 1.2 * scale,
                'labels': [
                  for (final label in _labels)
                    {
                      'address': label.nativeObject.address,
                      'size': label.fontSize,
                      'chinese': RegExp(r'[\u3400-\u9fff]')
                          .hasMatch(label.text ?? ''),
                    },
                ],
                'views': [
                  {'address': _badge.nativeObject.address, 'radius': 0.0},
                  {'address': _face.nativeObject.address, 'radius': 0.0},
                  {'address': _post.nativeObject.address, 'clip': true},
                ],
              });
          if (!_disposed && revision == _revision) {
            _styledRevision = revision;
            _layout();
          }
        })
        .catchError((Object error) {
          debugPrint('Native sign typography: $error');
        });
  }

  void _frame(na.View target, double x, double y, double w, double h) =>
      target.frame = na.Rectangle(x: x, y: y, width: w, height: h);

  void _center(na.Label label, double x, double y, double w, double h) {
    final textHeight = label.intrinsicSize.height;
    _frame(label, x, y + (h - textHeight) / 2, w, textHeight);
  }

  void _layout() {
    if (_disposed) return;
    if (_style == SignStyle.missing) {
      _layoutMissing();
    } else if (_style == SignStyle.travel) {
      _layoutTravel();
    } else {
      _layoutPanel();
    }
    view.preferredSize = na.Size(width: width, height: height);
    onLayout?.call();
  }

  void _layoutMissing() {
    final h = height;
    final side = math.max(
      h * 0.35,
      [
            _west,
            _east,
            _w,
            _e,
          ].map((label) => label.intrinsicSize.width).reduce(math.max) +
          scale,
    );
    final textWidth =
        math.max(
          _title.intrinsicSize.width,
          _bilingual ? _caption.intrinsicSize.width : 0.0,
        ) +
        h * 0.12;
    width = (textWidth + side * 2).ceilToDouble();
    final top = _bilingual ? h * 0.68 : h;
    _frame(_face, 0, 0, width, top);
    _center(_title, side, 0, textWidth, top);
    _center(_west, 0, 0, side, top);
    _center(_east, width - side, 0, side, top);
    _center(_caption, side, top, textWidth, h - top);
    _center(_w, 0, top, side, h - top);
    _center(_e, width - side, top, side, h - top);
  }

  void _layoutPanel() {
    final h = height;
    final edge = scale;
    final pad = h * 0.22;
    final faceHeight = h - edge * 2;
    final badgeHeight = h * 0.72;
    final badgeInset = (faceHeight - badgeHeight) / 2;
    final badgeBodyWidth = math.max(
      badgeHeight,
      _badgeNumber.intrinsicSize.width + scale * 4,
    );
    final badgeWidth = _badge.isVisible ? badgeBodyWidth + badgeInset : 0.0;
    final leftArrow = _west.isVisible
        ? math.max(h * 0.7, _west.intrinsicSize.width + scale * 2)
        : 0.0;
    final rightArrow = _east.isVisible
        ? math.max(h * 0.7, _east.intrinsicSize.width + scale * 2)
        : 0.0;
    final bodyWidth =
        math.max(
          _title.intrinsicSize.width,
          _caption.isVisible ? _caption.intrinsicSize.width : 0.0,
        ) +
        pad * 2;
    width = (badgeWidth + leftArrow + bodyWidth + rightArrow + edge * 2)
        .ceilToDouble();
    _frame(_face, edge, edge, width - edge * 2, faceHeight);
    final start = badgeWidth + leftArrow;
    final titleTop = _bilingual ? faceHeight * 0.04 : 0.0;
    final titleHeight = _bilingual ? faceHeight * 0.64 : faceHeight;
    _center(_title, start, titleTop, bodyWidth, titleHeight);
    _center(_caption, edge + start, h * 0.64, bodyWidth, h * 0.31);
    _center(_west, badgeWidth, 0, leftArrow, faceHeight);
    _center(_east, width - edge * 2 - rightArrow, 0, rightArrow, faceHeight);
    if (_badge.isVisible) {
      final bh = badgeHeight, bw = badgeBodyWidth;
      _frame(_badge, edge + badgeInset, edge + badgeInset, bw, bh);
      _center(_badgeNumber, 0, 0, bw, bh);
    }
  }

  void _layoutTravel() {
    final h = height;
    final side = math.max(
      h * 0.60,
      math.max(_east.intrinsicSize.width, _detail.intrinsicSize.width) +
          scale * 2,
    );
    final body =
        math.max(_title.intrinsicSize.width, _caption.intrinsicSize.width) +
        h * 0.3;
    width = (body + side).ceilToDouble();
    _frame(_post, width * 0.47, 0, h * 0.12, h);
    _frame(_face, 0, 0, width, h * 0.47);
    _frame(_badge, 0, h * 0.53, width, h * 0.47);
    _center(_title, 0, 0, body, h * 0.47);
    _center(_east, body, 0, side, h * 0.47);
    _center(_detail, 0, h * 0.53, side, h * 0.47);
    _center(_caption, side, h * 0.53, body, h * 0.47);
  }

  void dispose() {
    if (_disposed) return;
    _disposed = true;
    void release() {
      for (final item in _owned.reversed) {
        item.dispose();
      }
      _owned.clear();
    }

    if (Platform.isMacOS) {
      _styling.whenComplete(release);
    } else {
      release();
    }
  }
}
