import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

class _Window extends Window {
  _Window() : super.borrowed(0);

  final reports = <Rectangle>[];

  @override
  bool setMaximizeButtonBounds(Rectangle bounds) {
    reports.add(bounds);
    return true;
  }
}

Widget _host(Window window, {required double left, double width = 46}) =>
    Directionality(
      textDirection: TextDirection.ltr,
      child: Stack(
        children: [
          Positioned(
            left: left,
            top: 0,
            child: MaximizeButtonArea(
              window: window,
              child: SizedBox(width: width, height: 32),
            ),
          ),
        ],
      ),
    );

void expectRect(Rectangle r, double x, double y, double width, double height) {
  expect([r.x, r.y, r.width, r.height], [x, y, width, height]);
}

void main() {
  testWidgets('reports the button rectangle in content coordinates', (
    tester,
  ) async {
    final window = _Window();
    await tester.pumpWidget(_host(window, left: 600));
    await tester.pump();
    expect(window.reports, hasLength(1));
    expectRect(window.reports.single, 600, 0, 46, 32);
  });

  testWidgets('reports again only when the button moves or resizes', (
    tester,
  ) async {
    final window = _Window();
    await tester.pumpWidget(_host(window, left: 600));
    await tester.pump();
    await tester.pumpWidget(_host(window, left: 600));
    await tester.pump();
    expect(window.reports, hasLength(1));
    await tester.pumpWidget(_host(window, left: 500, width: 60));
    await tester.pump();
    expect(window.reports, hasLength(2));
    expectRect(window.reports.last, 500, 0, 60, 32);
  });

  testWidgets('clears the area when the button goes away', (tester) async {
    final window = _Window();
    await tester.pumpWidget(_host(window, left: 600));
    await tester.pump();
    await tester.pumpWidget(const SizedBox());
    expectRect(window.reports.last, 0, 0, 0, 0);
  });

  testWidgets('moves the area to another window', (tester) async {
    final first = _Window();
    final second = _Window();
    await tester.pumpWidget(_host(first, left: 600));
    await tester.pump();
    await tester.pumpWidget(_host(second, left: 600));
    await tester.pump();
    expectRect(first.reports.last, 0, 0, 0, 0);
    expectRect(second.reports.single, 600, 0, 46, 32);
  });
}
