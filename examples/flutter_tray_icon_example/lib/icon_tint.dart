import 'dart:ui' as ui;

import 'package:flutter/services.dart';

/// Recolour a monochrome image without losing its dimensions or alpha mask.
Future<Uint8List> tintIcon(Uint8List png, ui.Color color) async {
  final codec = await ui.instantiateImageCodec(png);
  final source = (await codec.getNextFrame()).image;
  codec.dispose();
  final recorder = ui.PictureRecorder();
  final canvas = ui.Canvas(recorder);
  canvas.drawImage(
    source,
    ui.Offset.zero,
    ui.Paint()..colorFilter = ui.ColorFilter.mode(color, ui.BlendMode.srcIn),
  );
  final picture = recorder.endRecording();
  final tinted = await picture.toImage(source.width, source.height);
  picture.dispose();
  source.dispose();
  try {
    final data = await tinted.toByteData(format: ui.ImageByteFormat.png);
    return data!.buffer.asUint8List();
  } finally {
    tinted.dispose();
  }
}
