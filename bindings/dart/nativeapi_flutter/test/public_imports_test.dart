import 'dart:ui';

import 'package:flutter_test/flutter_test.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

void main() {
  test('Flutter value types resolve alongside nativeapi_flutter', () {
    expect(const TextRange.collapsed(2).isCollapsed, isTrue);
    expect(const Size(20, 10).toNative().height, 10);
  });
}
