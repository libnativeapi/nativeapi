import 'dart:ui' as ui;

import 'package:flutter_test/flutter_test.dart';
import 'package:tray_icon_example/icon_tint.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test(
    'tint preserves image dimensions and transparent/partial alpha pixels',
    () async {
      final recorder = ui.PictureRecorder();
      final canvas = ui.Canvas(recorder);
      canvas.drawRect(
        const ui.Rect.fromLTWH(0, 0, 1, 1),
        ui.Paint()..color = const ui.Color(0xFFFFFFFF),
      );
      canvas.drawRect(
        const ui.Rect.fromLTWH(1, 0, 1, 1),
        ui.Paint()..color = const ui.Color(0x80FFFFFF),
      );
      final picture = recorder.endRecording();
      final image = await picture.toImage(3, 1);
      picture.dispose();
      final png = await image.toByteData(format: ui.ImageByteFormat.png);
      image.dispose();

      for (final color in [
        const ui.Color(0xFF000000),
        const ui.Color(0xFF2F7DE1),
      ]) {
        final tinted = await tintIcon(png!.buffer.asUint8List(), color);
        final codec = await ui.instantiateImageCodec(tinted);
        final frame = (await codec.getNextFrame()).image;
        codec.dispose();
        expect((frame.width, frame.height), (3, 1));
        final pixels = (await frame.toByteData(
          format: ui.ImageByteFormat.rawStraightRgba,
        ))!.buffer.asUint8List();
        frame.dispose();
        expect(pixels[3], 255);
        expect(pixels[7], closeTo(128, 1));
        expect(pixels[11], 0);
        expect(pixels.sublist(0, 3), [
          (color.toARGB32() >> 16) & 255,
          (color.toARGB32() >> 8) & 255,
          color.toARGB32() & 255,
        ]);
      }
    },
  );
}
