import 'dart:math' as math;

import 'package:flutter/widgets.dart';

import 'sign_style.dart';

/// Lightweight template thumbnails. The live macOS preview uses PlatformView.
class SignArt extends StatelessWidget {
  const SignArt({
    super.key,
    required this.style,
    this.content,
    this.green = false,
    this.english = true,
    this.right = true,
    this.height = 24,
  });

  final SignStyle style;
  final SignContent? content;
  final bool green, english, right;
  final double height;

  @override
  Widget build(BuildContext context) {
    final painter = _SignPainter(
      style,
      content ?? style.defaults,
      green,
      english,
      right,
    );
    return Semantics(
      label: (content ?? style.defaults).heading(style),
      child: SizedBox(
        height: height,
        child: FittedBox(
          fit: BoxFit.contain,
          child: CustomPaint(size: Size(painter.width, 100), painter: painter),
        ),
      ),
    );
  }
}

class _SignPainter extends CustomPainter {
  _SignPainter(this.style, this.content, this.green, this.english, this.right);
  final SignStyle style;
  final SignContent content;
  final bool green, english, right;
  static const light = Color(0xFFFAFAF5), ink = Color(0xFF141E25);
  static const blue = Color(0xFF184170), roadGreen = Color(0xFF00694E);
  static const brown = Color(0xFF683E24), ochre = Color(0xFFE0B766);
  bool get bilingual =>
      style == SignStyle.missing || style == SignStyle.travel || english;
  double get main =>
      (bilingual ? 35.0 : 46.0) *
      (style == SignStyle.welcome
          ? 1.12
          : style == SignStyle.guide
          ? 1.1
          : 1);
  String get caption => switch (style) {
    SignStyle.missing =>
      'WoZai${content.secondary.split(RegExp(r'[ -]+')).map((w) => w.isEmpty ? '' : w[0].toUpperCase() + w.substring(1).toLowerCase()).join()}HenXiangNi',
    SignStyle.welcome => 'WELCOME TO ${content.secondary.toUpperCase()}',
    SignStyle.guide =>
      '${content.secondary.toUpperCase()}  /  ${content.value}',
    SignStyle.travel => content.secondary,
  };
  TextPainter text(String value, double size, Color color) => TextPainter(
    text: TextSpan(
      text: value,
      style: TextStyle(
        fontSize: size,
        fontWeight: FontWeight.w600,
        color: color,
      ),
    ),
    textDirection: TextDirection.ltr,
    maxLines: 1,
  )..layout();
  double get side => switch (style) {
    SignStyle.missing => 35,
    SignStyle.travel => 60,
    SignStyle.guide => 70,
    _ => 0,
  };
  double get badge => style == SignStyle.guide ? 82 : 0;
  double get body => style == SignStyle.travel
      ? math.max(
              text(content.primary, 28, light).width,
              text(caption, 26, ink).width,
            ) +
            30
      : math.max(
              text(content.heading(style), main, light).width,
              bilingual ? text(caption, 18.5, light).width : 0,
            ) +
            (style == SignStyle.missing ? 12 : 44);
  double get width =>
      body +
      (style == SignStyle.missing
          ? side * 2
          : side + badge + (style == SignStyle.travel ? 0 : 8));

  @override
  void paint(Canvas canvas, Size size) {
    final color = style == SignStyle.guide
        ? brown
        : style == SignStyle.travel || green
        ? roadGreen
        : blue;
    void rect(double x, double y, double w, double h, Color fill) =>
        canvas.drawRect(Rect.fromLTWH(x, y, w, h), Paint()..color = fill);
    void label(
      String value,
      double x,
      double y,
      double font, [
      Color fill = light,
    ]) {
      final p = text(value, font, fill);
      p.paint(canvas, Offset(x - p.width / 2, y - p.height / 2));
    }

    rect(0, 0, width, 100, light);
    if (style == SignStyle.missing) {
      rect(0, 0, width, 68, color);
      label(content.heading(style), side + body / 2, 34, main);
      label(right ? '东' : '西', side / 2, 34, main * .66);
      label(right ? '西' : '东', width - side / 2, 34, main * .66);
      label(caption, side + body / 2, 84, 18.5, ink);
      label(right ? 'E' : 'W', side / 2, 84, 18.5, ink);
      label(right ? 'W' : 'E', width - side / 2, 84, 18.5, ink);
    } else if (style == SignStyle.travel) {
      rect(width * .47, 0, 12, 100, brown);
      rect(0, 0, width, 47, color);
      rect(0, 53, width, 47, ochre);
      label(content.primary, body / 2, 23.5, 28);
      label(right ? '➜' : '←', body + side / 2, 23.5, 31);
      label(right ? '←' : '➜', side / 2, 76.5, 31, ink);
      label(content.secondary, side + body / 2, 76.5, 26, ink);
    } else {
      rect(4, 4, width - 8, 92, color);
      final start = badge + (!right ? side : 0) + 4;
      label(
        content.heading(style),
        start + body / 2,
        bilingual ? 35 : 50,
        main,
      );
      if (bilingual) label(caption, start + body / 2, 79.5, 18.5);
      if (side > 0) {
        label(
          right ? '➜' : '←',
          right ? width - 4 - side / 2 : 4 + badge + side / 2,
          46,
          50,
        );
      }
      if (badge > 0) {
        rect(14, 14, 72, 72, light);
        label('▲', 50, 50, 34, color);
      }
    }
  }

  @override
  bool shouldRepaint(_SignPainter old) =>
      old.style != style ||
      old.content != content ||
      old.green != green ||
      old.english != english ||
      old.right != right;
}
