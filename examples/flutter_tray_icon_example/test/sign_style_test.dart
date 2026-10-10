import 'package:flutter_test/flutter_test.dart';
import 'package:tray_icon_example/signs/sign_art.dart';
import 'package:tray_icon_example/signs/sign_style.dart';
import 'package:flutter/widgets.dart';

void main() {
  test('retained templates have valid, independent defaults', () {
    expect(SignStyle.values.map((style) => style.name), [
      'missing',
      'welcome',
      'travel',
      'guide',
    ]);
    for (final style in SignStyle.values) {
      expect(style.defaults.validate(style), isNull);
    }
    expect(SignStyle.missing.hasSubtitleToggle, isFalse);
    expect(SignStyle.travel.hasSubtitleToggle, isFalse);
    expect(SignStyle.guide.hasSubtitleToggle, isTrue);
  });

  test('invalid edits leave clear English validation feedback', () {
    expect(
      const SignContent('', 'SHANG HAI').validate(SignStyle.missing),
      contains('Place'),
    );
    expect(
      const SignContent('上海', '上海').validate(SignStyle.missing),
      contains('Pinyin'),
    );
    expect(
      const SignContent('上海', 'SHANG\nHAI').validate(SignStyle.missing),
      contains('line breaks'),
    );
    expect(
      const SignContent('观景台', 'VIEWPOINT', '').validate(SignStyle.guide),
      contains('Distance'),
    );
    expect(
      SignContent('上' * 11, 'SHANG HAI').validate(SignStyle.missing),
      contains('10 characters'),
    );
  });

  testWidgets('four compact thumbnails fit a single row', (tester) async {
    await tester.pumpWidget(
      Directionality(
        textDirection: TextDirection.ltr,
        child: Center(
          child: SizedBox(
            width: 400,
            child: Row(
              children: [
                for (final style in SignStyle.values)
                  Expanded(child: SignArt(style: style)),
              ],
            ),
          ),
        ),
      ),
    );
    expect(tester.takeException(), isNull);
    final rects = [
      for (final element in find.byType(SignArt).evaluate())
        tester.getRect(find.byWidget(element.widget)),
    ];
    expect(rects.map((rect) => rect.top).toSet().length, 1);
    expect(
      rects.every((rect) => rect.height == 24 && rect.width <= 100),
      isTrue,
    );
  });
}
